/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 * https://github.com/jcorporation/myGPIOd
 *
 * myGPIOd is based on the gpiomon tool from
 * https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/
 */

#include "compile_time.h"
#include "dist/sds/sds.h"
#include "input/input.h"
#include "lib/list.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/gpio/chip.h"
#include "mygpiod/gpio/input.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/config/config.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/sds_extras.h"
#include "mygpiod/server_http/httpd.h"
#include "mygpiod/server_socket/socket.h"

#ifdef MYGPIOD_ENABLE_ACTION_LUA
    #include "mygpiod/lua/luavm.h"
#endif

#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef MYGPIOD_ENABLE_ASAN
const char *__asan_default_options(void) {
    return "abort_on_error=1:fast_unwind_on_malloc=0:detect_stack_use_after_return=1";
}
#endif

#ifdef MYGPIOD_ENABLE_TSAN
const char *__asan_default_options(void) {
    return "abort_on_error=1";
}
#endif

#ifdef MYGPIOD_ENABLE_UBSAN
const char *__asan_default_options(void) {
    return "abort_on_error=1:print_stacktrace=1";
}
#endif

int main(int argc, char **argv) {
    // Set initial states
    int rc = EXIT_SUCCESS;
    logline = sdsempty();
    log_init();
    umask(0077);  // Only owner should have rw access

    // Handle command line parameter
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        printf("myGPIOD %s\n", MYGPIO_VERSION);
        printf("Usage: mygpiod [configuration file]\n");
        printf("Default configuration file: /etc/mygpiod.conf\n");
        return 1;
    }

    // First argument is the configuration file
    sds config_file = argc == 2
        ? sdsnew(argv[1])
        : sdsnew("/etc/mygpiod.conf");

    // Startup notice
    MYGPIOD_LOG_NOTICE("Starting myGPIOd %s", MYGPIO_VERSION);
    MYGPIOD_LOG_NOTICE("https://github.com/jcorporation/myGPIOd");
    MYGPIOD_LOG_NOTICE("libgpiod %s", gpiod_api_version());

    // Read config
    struct t_config *config = get_config(config_file);
    FREE_SDS(config_file);
    if (config == NULL) {
        rc = EXIT_FAILURE;
        goto out;
    }

    // set loglevel
    #ifndef MYGPIOD_DEBUG
        set_loglevel(config->loglevel);
    #endif

    // open syslog connection
    if (config->syslog == true &&
        log_type != LOG_TO_SYSTEMD)
    {
        openlog(MYGPIOD_NAME, LOG_CONS, LOG_DAEMON);
        log_type = LOG_TO_SYSLOG;
    }

    // Set output buffers
    if (setvbuf(stdout, NULL, _IOLBF, 0) != 0 ||
        setvbuf(stderr, NULL, _IOLBF, 0) != 0)
    {
        MYGPIOD_LOG_EMERG("Could not set stdout and stderr buffer");
        rc = EXIT_FAILURE;
        goto out;
    }

    #ifdef MYGPIOD_ENABLE_ACTION_LUA
        if (luavm_init(config) == false) {
            rc = EXIT_FAILURE;
            goto out;
        }
    #endif

    // init struct for event polling
    struct t_poll_fds poll_fds;
    memset(&poll_fds, 0, sizeof(poll_fds));
    update_pollfds = true;

    // open the chip, set output gpios and request input gpios
    if (sdslen(config->chip_path) > 0) {
        if (gpio_open_chip(config) == false ||
            gpio_set_outputs(config) == false ||
            gpio_request_inputs(config, &poll_fds) == false)
        {
            rc = EXIT_FAILURE;
            goto out;
        }
    }
    else {
        MYGPIOD_LOG_INFO("No GPIO chip configured");
        config_gpios_clear(config);
    }

    // add input fds
    inputs_open(config, &poll_fds);

    // add signal fd
    event_poll_fd_add(&poll_fds, config->signal_fd, PFD_TYPE_SIGNAL, POLLIN | POLLPRI);

    // create server socket
    int server_fd = server_socket_create(config);
    if (server_fd == -1) {
        rc = EXIT_FAILURE;
        goto out;
    }
    event_poll_fd_add(&poll_fds, server_fd, PFD_TYPE_CONNECT, POLLIN | POLLPRI);

    // create http server
    if (config->http_port > 0) {
        config->httpd = httpd_start(config);
        if (config->httpd == NULL) {
            MYGPIOD_LOG_EMERG("Failure starting http server.");
            rc = EXIT_FAILURE;
            goto out;
        }
        const union MHD_DaemonInfo *httpd_fd = MHD_get_daemon_info(config->httpd, MHD_DAEMON_INFO_EPOLL_FD);
        if (httpd_fd == NULL ||
            httpd_fd->epoll_fd == -1)
        {
            MYGPIOD_LOG_EMERG("Failure getting MHD epoll fd.");
            rc = EXIT_FAILURE;
            goto out;
        }
        event_poll_fd_add(&poll_fds, httpd_fd->epoll_fd, PFD_TYPE_HTTPD, POLLIN | POLLOUT | POLLPRI);
    }

    // main event handling loop
    MYGPIOD_LOG_INFO("Entering event handling loop");
    MYGPIOD_LOG_INFO("Monitoring %u gpios", config->gpios_in.length);

    // save initial number of fds to poll
    unsigned pfd_len_init = poll_fds.len;
    while (true) {
        if (update_pollfds == true) {
            // reset poll_fds length and re-add the timer and client fds
            poll_fds.len = pfd_len_init;
            event_add_gpio_in_timer_fds(config, &poll_fds);
            event_add_gpio_out_timer_fds(config, &poll_fds);
            event_add_client_fds(config, &poll_fds);
            update_pollfds = false;
        }

        // Poll
        MYGPIOD_LOG_DEBUG("Polling %u fds", poll_fds.len);
        // Use timeout from MHD
        // This is required for MHD connection suspend and resume
        int timeout;
        MHD_UNSIGNED_LONG_LONG to;
        if (MHD_get_timeout (config->httpd, &to) != MHD_YES) {
            timeout = -1;
        }
        else {
            timeout = (to < INT_MAX - 1) ? (int) to : (INT_MAX - 1);
        }
        int cnt = poll(poll_fds.fd, poll_fds.len, timeout);
        if (cnt < 0) {
            MYGPIOD_LOG_ERROR("Failure polling fds");
            rc = EXIT_FAILURE;
            goto out;
        }
        // MHD must be always called, even on poll timeout
        if (MHD_run(config->httpd) != MHD_YES) {
            MYGPIOD_LOG_ERROR("Failure running MHD");
            rc = EXIT_FAILURE;
            goto out;
        }
        // No waiting events
        if (cnt == 0) {
            MYGPIOD_LOG_DEBUG("Poll timeout");
            continue;
        }
        // Read and delegate events
        if (event_read_delegate(config, &poll_fds) == false) {
            break;
        }
    }

out:
    if (config != NULL) {
        config_clear(config);
        FREE_PTR(config);
    }
    if (rc == EXIT_SUCCESS) {
        MYGPIOD_LOG_INFO("Exiting gracefully, thank you for using myGPIOd");
    }
    FREE_SDS(logline);
    return rc;
}

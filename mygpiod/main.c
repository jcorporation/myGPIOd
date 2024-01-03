/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 * https://github.com/jcorporation/myGPIOd
 *
 * myGPIOd is based on the gpiomon tool from
 * https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/
 */

#include "compile_time.h"
#include "dist/sds/sds.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/gpio/chip.h"
#include "mygpiod/gpio/input.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/lib/config.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/util.h"
#include "mygpiod/server/socket.h"

#include <poll.h>
#include <stdio.h>
#include <string.h>
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
    int rc = EXIT_SUCCESS;

    log_on_tty = isatty(fileno(stdout))
        ? true
        : false;
    log_to_syslog = false;

    #ifdef MYGPIOD_DEBUG
        set_loglevel(LOG_DEBUG);
    #else
        set_loglevel(CFG_LOGLEVEL);
    #endif

    MYGPIOD_LOG_NOTICE("Starting myGPIOd %s", MYGPIO_VERSION);
    MYGPIOD_LOG_NOTICE("https://github.com/jcorporation/myGPIOd");
    MYGPIOD_LOG_NOTICE("libgpiod %s", gpiod_api_version());

    // Handle command line parameter
    sds config_file = argc == 2 && strncmp(argv[1], "/", 1) == 0
        ? sdsnew(argv[1])
        : sdsnew("/etc/mygpiod.conf");

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
    if (config->syslog == true) {
        openlog(MYGPIOD_NAME, LOG_CONS, LOG_DAEMON);
        log_to_syslog = true;
    }

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
            goto out;
        }
    }

    // add signal fd
    event_poll_fd_add(&poll_fds, config->signal_fd, PFD_TYPE_SIGNAL, POLLIN | POLLPRI);

    // create server socket
    int server_fd = server_socket_create(config);
    if (server_fd == -1) {
        goto out;
    }
    event_poll_fd_add(&poll_fds, server_fd, PFD_TYPE_CONNECT, POLLIN | POLLPRI);

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

        // poll
        MYGPIOD_LOG_DEBUG("Polling %u fds", poll_fds.len);
        int cnt = poll(poll_fds.fd, poll_fds.len, -1);
        if (cnt < 0) {
            MYGPIOD_LOG_ERROR("Failure polling fds");
            rc = EXIT_FAILURE;
            goto out;
        }
        if (cnt == 0) {
            MYGPIOD_LOG_DEBUG("Poll timeout");
            continue;
        }

        // read and delegate events
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
    return rc;
}

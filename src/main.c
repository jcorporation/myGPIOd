/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 * https://github.com/jcorporation/myGPIOd
 *
 * myGPIOd is based on the gpiomon tool from
 * https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/
 */

#include "compile_time.h"
#include "config.h"
#include "event.h"
#include "gpio.h"
#include "log.h"
#include "timer.h"

#include <errno.h>
#include <gpiod.h>
#include <poll.h>
#include <stdint.h>
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

    MYGPIOD_LOG_NOTICE("Starting myGPIOd %s", MYGPIOD_VERSION);
    MYGPIOD_LOG_NOTICE("https://github.com/jcorporation/myGPIOd");
    MYGPIOD_LOG_NOTICE("libgpiod %s", gpiod_version_string());

    #ifdef MYGPIOD_DEBUG
        set_loglevel(LOG_DEBUG);
    #else
        set_loglevel(CFG_LOGLEVEL);
    #endif

    // Handle command line parameter
    char *config_file = NULL;
    if (argc == 2 && strncmp(argv[1], "/", 1) == 0) {
        config_file = strdup(argv[1]);
    }
    else {
        config_file = strdup("/etc/mygpiod.conf");
    }

    // Read config
    struct t_config *config = get_config(config_file);
    free(config_file);
    if (config == NULL) {
        rc = EXIT_FAILURE;
        goto out;
    }

    if (config->gpios_in.length == 0) {
        MYGPIOD_LOG_ERROR("No gpios for monitoring configured");
        rc = EXIT_FAILURE;
        goto out;
    }

    // Set loglevel
    #ifdef MYGPIOD_DEBUG
        set_loglevel(LOG_DEBUG);
    #else
        set_loglevel(config->loglevel);
    #endif

    if (config->syslog == true) {
        openlog(MYGPIOD_NAME, LOG_CONS, LOG_DAEMON);
        log_to_syslog = true;
    }

    // open the chip
    MYGPIOD_LOG_INFO("Opening chip \"%s\"", config->chip_name);
    config->chip = gpiod_chip_open_lookup(config->chip_name);
    if (config->chip == NULL) {
        MYGPIOD_LOG_ERROR("Error opening chip");
        rc = EXIT_FAILURE;
        goto out;
    }

    // output gpios
    if (gpio_set_outputs(config) == false) {
        goto out;
    }

    // input gpios
    if (gpio_request_inputs(config) == false) {
        goto out;
    }

    // Main event handling loop
    MYGPIOD_LOG_INFO("Entering event handling loop");
    MYGPIOD_LOG_INFO("Monitoring %u gpios", config->gpios_in.length);

    #define MAX_FDS (GPIOD_LINE_BULK_MAX_LINES * 2 + 1)
    struct t_list_node *current;
    while (true) {
        struct pollfd pfds[MAX_FDS];
        int pfds_type[MAX_FDS];
        unsigned pfd_count = 0;

        // add gpio fds
        current = config->gpios_in.head;
        while (current != NULL) {
            struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
            pfd_count = poll_fd_add(&pfds[pfd_count], &pfds_type[pfd_count], pfd_count,
                data->fd, POLLIN | POLLPRI, PFD_TYPE_GPIO);
            current = current->next;
        }

        // add timer fds
        current = config->gpios_in.head;
        while (current != NULL) {
            struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
            if (data->timer_fd > 0) {
                pfd_count = poll_fd_add(&pfds[pfd_count], &pfds_type[pfd_count], pfd_count,
                    data->timer_fd, POLLIN, PFD_TYPE_TIMER);
            }
            current = current->next;
        }

        // add signal fd
        pfd_count = poll_fd_add(&pfds[pfd_count], &pfds_type[pfd_count], pfd_count,
            config->signal_fd, POLLIN | POLLPRI, PFD_TYPE_SIGNAL);

        // poll
        int cnt = poll(pfds, pfd_count, -1);
        if (cnt < 0) {
            MYGPIOD_LOG_ERROR("Failure polling fds");
            rc = EXIT_FAILURE;
            goto out;
        }
        if (cnt == 0) {
            MYGPIOD_LOG_DEBUG("Poll timeout");
            continue;
        }

        // get event
        for (unsigned i = 0; i < pfd_count; i++) {
            if (pfds[i].revents) {
                bool rv = false;
                switch(pfds_type[i]) {
                    case PFD_TYPE_GPIO:
                        rv = gpio_handle_event(config, i);
                        break;
                    case PFD_TYPE_TIMER:
                        rv = timer_handle_event(&pfds[i].fd, config, i);
                        break;
                    case PFD_TYPE_SIGNAL:
                        MYGPIOD_LOG_DEBUG("%u: Signal event detected", i);
                        rc = EXIT_SUCCESS;
                        break;
                }
                if (rv == false) {
                    goto out;
                }
            }
        }
    }

out:
    if (config != NULL) {
        config_clear(config);
        free(config);
    }
    if (rc == EXIT_SUCCESS) {
        MYGPIOD_LOG_INFO("Exiting gracefully, thank you for using myGPIOd");
    }
    return rc;
}

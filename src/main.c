/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 * https://github.com/jcorporation/myGPIOd
 *
 * myGPIOd is based on the gpiomon tool from
 * https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/
 */

#include "compile_time.h"
#include "action.h"
#include "config.h"
#include "log.h"
#include "util.h"

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
    unsigned i;
    struct gpiod_line *line;

    log_on_tty = isatty(fileno(stdout))
        ? true
        : false;
    log_to_syslog = false;
    
    #ifdef MYGPIOD_DEBUG
        set_loglevel(LOG_DEBUG);
    #else
        set_loglevel(CFG_LOGLEVEL);
    #endif

    MYGPIOD_LOG_NOTICE("Starting myGPIOd %s", MYGPIOD_VERSION);
    MYGPIOD_LOG_NOTICE("https://github.com/jcorporation/myGPIOd");
    MYGPIOD_LOG_NOTICE("libgpiod %s", gpiod_version_string());

    // Handle commandline parameter
    char *config_file = NULL;
    if (argc == 2 && strncmp(argv[1], "/", 1) == 0) {
        config_file = strdup(argv[1]);
    }
    else {
        config_file = strdup("/etc/mygpiod.conf");
    }

    // Read config
    MYGPIOD_LOG_INFO("Reading \"%s\"", config_file);
    struct t_config *config = config_new();
    if (config == NULL) {
        MYGPIOD_LOG_EMERG("Out of memory");
        free(config_file);
        rc = EXIT_FAILURE;
        goto out;
    }
    if (config_read(config, config_file) == false) {
        free(config_file);
        rc = EXIT_FAILURE;
        goto out;
    }
    free(config_file);

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
    if (config->gpios_out.length > 0) {
        int gpios_out_values[GPIOD_LINE_BULK_MAX_LINES];
        struct gpiod_line_bulk bulk_out;
        gpiod_line_bulk_init(&bulk_out);
        struct t_list_node *current = config->gpios_out.head;
        i = 0;
        while (current != NULL) {
            line = gpiod_chip_get_line(config->chip, current->gpio);
            if (line == NULL) {
                MYGPIOD_LOG_ERROR("Error getting gpio \"%u\"", current->gpio);
                rc = EXIT_FAILURE;
                goto out;
            }
            struct t_gpio_node_out *data = (struct t_gpio_node_out *)current->data;
            MYGPIOD_LOG_INFO("Setting gpio \"%u\" as output to value \"%s\"", current->gpio, lookup_gpio_value(data->value));
            gpiod_line_bulk_add(&bulk_out, line);
            gpios_out_values[i] = data->value;
            current = current->next;
            i++;
        }

        // set values
        int req_flags = line_request_flags(config->active_low, 0);
        errno = 0;
        int rv = gpiod_line_request_bulk_output_flags(&bulk_out, MYGPIOD_NAME,
            req_flags, gpios_out_values);
        if (rv == -1) {
            MYGPIOD_LOG_ERROR("Error setting gpios: %s", strerror(errno));
            rc = EXIT_FAILURE;
            goto out;
        }
    }

    // input gpios
    struct gpiod_line_bulk bulk_in;
    gpiod_line_bulk_init(&bulk_in);
    struct t_list_node *current = config->gpios_in.head;
    while (current != NULL) {
        line = gpiod_chip_get_line(config->chip, current->gpio);
        if (line == NULL) {
            MYGPIOD_LOG_ERROR("Error getting gpio \"%u\"", current->gpio);
            rc = EXIT_FAILURE;
            goto out;
        }
        struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
        MYGPIOD_LOG_INFO("Setting gpio \"%u\" as input, monitoring event: %s", current->gpio, lookup_event_request(data->request_event));
        gpiod_line_bulk_add(&bulk_in, line);
        current = current->next;
    }

    // set request flags
    struct gpiod_line_request_config conf;
    conf.flags = line_request_flags(config->active_low, config->bias);
    conf.consumer = MYGPIOD_NAME;
    conf.request_type = config->event_request;

    // request the gpios
    errno = 0;
    int rv = gpiod_line_request_bulk(&bulk_in, &conf, NULL);
    if (rv == -1) {
        MYGPIOD_LOG_ERROR("Error requesting gpios: %s", strerror(errno));
        rc = EXIT_FAILURE;
        goto out;
    }

    // get fds
    current= config->gpios_in.head;
    i = 0;
    while (current != NULL) {
        struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
        line = gpiod_line_bulk_get_line(&bulk_in, i);
        data->fd = gpiod_line_event_get_fd(line);
        current = current->next;
        i++;
    }

    // Main event handling loop
    MYGPIOD_LOG_INFO("Entering event handling loop");
    MYGPIOD_LOG_INFO("Monitoring %u gpios", config->gpios_in.length);

    while (true) {
        struct pollfd pfds[GPIOD_LINE_BULK_MAX_LINES * 2 + 1];
        unsigned pfd_count = 0;

        // add gpio fds
        current = config->gpios_in.head;
        while (current != NULL) {
            struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
            pfds[pfd_count].fd = data->fd;
            pfds[pfd_count].events = POLLIN | POLLPRI;
            pfd_count++;
            current = current->next;
        }

        // add timer fds
        current = config->gpios_in.head;
        while (current != NULL) {
            struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
            if (data->timer_fd > 0) {
                pfds[pfd_count].fd = data->timer_fd;
                pfds[pfd_count].events = POLLIN;
                pfd_count++;
            }
            current = current->next;
        }

        // add signal fds
        pfds[pfd_count].fd = config->signal_fd;
        pfds[pfd_count].events = POLLIN | POLLPRI;
        pfd_count++;

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

        // gpio event
        for (i = 0; i < config->gpios_in.length; i++) {
            if (pfds[i].revents) {
                MYGPIOD_LOG_DEBUG("%u: Gpio event detected", i);
                line = gpiod_line_bulk_get_line(&bulk_in, i);
                struct gpiod_line_event event;
                rv = gpiod_line_event_read(line, &event);
                if (rv < 0) {
                    rc = EXIT_FAILURE;
                    goto out;
                }
                struct t_list_node *node = list_node_at(&config->gpios_in, i);
                struct t_gpio_node_in *data = (struct t_gpio_node_in *)node->data;
                // abort pending long press event
                action_delay_abort(data);
                action_handle(node->gpio, &event.ts, event.event_type, data);
            }
        }

        // timerfd event
        for (; i < pfd_count - 1; i++) {
            if (pfds[i].revents & POLLIN) {
                MYGPIOD_LOG_DEBUG("%u: Long press timer event detected", i);
                uint64_t exp;
                ssize_t s = read(pfds[i].fd, &exp, sizeof(uint64_t));
                if (s == sizeof(uint64_t) && exp > 1) {
                    struct t_list_node *node = list_node_at(&config->gpios_in, i);
                    struct t_gpio_node_in *data = (struct t_gpio_node_in *)node->data;
                    action_execute_delayed(node->gpio, data, config);
                }
            }
        }

        // signalfd event
        if (pfds[i].revents) {
            MYGPIOD_LOG_DEBUG("%u: Signal event detected", i);
            rc = EXIT_SUCCESS;
            goto out;
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

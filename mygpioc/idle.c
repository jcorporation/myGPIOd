/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "mygpioc/idle.h"

#include "mygpioc/util.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Sends the idle command and waits for idle events
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_idle(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    int timeout = -1;
    if (argc == option_index + 1) {
        if (parse_int(argv[option_index], &timeout, -1, INT_MAX) == false) {
            fprintf(stderr, "Invalid timeout\n");
            return EXIT_FAILURE;
        }
    }
    verbose_printf("Sending idle");
    if (mygpio_send_idle(conn) == true) {
        verbose_printf("In idle mode");
    }
    verbose_printf("Waiting for idle events");
    if (mygpio_wait_idle(conn, timeout) == true) {
        verbose_printf("Events waiting");
        struct t_mygpio_idle_event *event;
        while ((event = mygpio_recv_idle_event(conn)) != NULL) {
            printf("GPIO %u, event %u, timestamp %llu\n", event->gpio, event->event, (unsigned long long)event->timestamp);
            mygpio_free_idle_event(event);
        }
        mygpio_response_end(conn);
        return EXIT_SUCCESS;
    }
    verbose_printf("No events waiting");
    mygpio_response_end(conn);
    return EXIT_FAILURE;
}

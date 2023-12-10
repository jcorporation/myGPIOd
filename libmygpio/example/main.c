/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "libmygpio/include/libmygpio/libmygpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    printf("Connecting to myGPIOd\n");
    struct t_mygpio_connection *conn = mygpio_connection_new(CFG_SOCKET_PATH, 5000);
    if (conn == NULL) {
        printf("Out of memory\n");
        return EXIT_FAILURE;
    }

    if (mygpio_connection_get_state(conn) != MYGPIO_STATE_OK) {
        printf("Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_connection_free(conn);
        return EXIT_FAILURE;
    }

    const unsigned *version = mygpio_connection_get_version(conn);
    printf("Connected, server version %u.%u.%u\n", version[0], version[1], version[2]);

    printf("Sending idle\n");
    if (mygpio_send_idle(conn) == true) {
        printf("In idle mode\n");
    }

    printf("Waiting for idle events\n");
    if (mygpio_wait_idle(conn, 5000) == true) {
        printf("Events waiting\n");
        struct t_mygpio_idle_event *event;
        while ((event = mygpio_recv_idle_event(conn)) != NULL) {
            printf("GPIO %u, event %u, timestamp %llu\n", event->gpio, event->event, (unsigned long long)event->timestamp);
            mygpio_free_idle_event(event);
        }
    }
    mygpio_response_end(conn);

    printf("Sending noidle\n");
    if (mygpio_send_noidle(conn) == true) {
        printf("Exited idle mode\n");
    }
    mygpio_response_end(conn);

    printf("Sending gpiolist\n");
    if (mygpio_gpiolist(conn) == true) {
        struct t_mygpio_gpio_conf *gpio_conf;
        printf("Retrieving gpio config\n");
        while ((gpio_conf = mygpio_recv_gpio_conf(conn)) != NULL) {
            printf("GPIO %u, mode %u\n", gpio_conf->gpio, gpio_conf->mode);
            mygpio_free_gpio_conf(gpio_conf);
        }
    }
    else {
        printf("Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_connection_clear_error(conn);
    }
    mygpio_response_end(conn);

    printf("Sending gpioget 5\n");
    enum mygpio_gpio_value value = mygpio_gpioget(conn, 5);
    if (value == MYGPIO_GPIO_VALUE_UNKNOWN) {
        printf("Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_connection_clear_error(conn);
    }
    else {
        printf("Value: %u\n", value);
    }
    mygpio_response_end(conn);

    printf("Sending gpioset 6 1\n");
    if (mygpio_gpioset(conn, 6, 1) == false) {
        printf("Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_connection_clear_error(conn);
    }
    mygpio_response_end(conn);

    printf("Closing connection\n");
    mygpio_connection_free(conn);

    return EXIT_SUCCESS;
}

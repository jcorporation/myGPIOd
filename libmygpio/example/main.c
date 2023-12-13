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

    // Create a connection struct and connect to /run/mygpiod/socket
    printf("Connecting to myGPIOd\n");
    struct t_mygpio_connection *conn = mygpio_connection_new(CFG_SOCKET_PATH, 5000);
    if (conn == NULL) {
        printf("Out of memory\n");
        return EXIT_FAILURE;
    }

    // Check the connection state
    if (mygpio_connection_get_state(conn) != MYGPIO_STATE_OK) {
        printf("Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_connection_free(conn);
        return EXIT_FAILURE;
    }

    // Retrieve the server version
    const unsigned *version = mygpio_connection_get_version(conn);
    printf("Connected, server version %u.%u.%u\n", version[0], version[1], version[2]);

    // Enter the myGPIOd idle mode to wait for events
    // All timeouts are disabled
    printf("Sending idle\n");
    if (mygpio_send_idle(conn) == true) {
        printf("In idle mode\n");
    }

    // Wait 5 seconds for an idle event
    printf("Waiting for idle events\n");
    if (mygpio_wait_idle(conn, 5000) == true) {
        printf("Events occurred\n");
        struct t_mygpio_idle_event *event;
        // Retrieve the list of events
        while ((event = mygpio_recv_idle_event(conn)) != NULL) {
            printf("GPIO %u, event %u, timestamp %llu ms\n",
                mygpio_idle_event_get_gpio(event),
                mygpio_idle_event_get_event(event),
                (unsigned long long)mygpio_idle_event_get_timestamp_ms(event)
            );
            mygpio_free_idle_event(event);
        }
    }
    else {
        printf("No events occurred\n");
    }
    mygpio_response_end(conn);

    // Exit the idle mode to send commands to myGPIOd
    printf("Sending noidle\n");
    if (mygpio_send_noidle(conn) == true) {
        printf("Exited idle mode\n");
    }
    mygpio_response_end(conn);

    // List the modes of the configured GPIOs
    printf("Sending gpiolist\n");
    if (mygpio_gpiolist(conn) == true) {
        struct t_mygpio_gpio_conf *gpio_conf;
        printf("Retrieving gpio config\n");
        while ((gpio_conf = mygpio_recv_gpio_conf(conn)) != NULL) {
            printf("GPIO %u, mode %u\n", 
                mygpio_gpio_conf_get_gpio(gpio_conf),
                mygpio_gpio_conf_get_mode(gpio_conf)
            );
            mygpio_free_gpio_conf(gpio_conf);
        }
    }
    else {
        printf("Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_connection_clear_error(conn);
    }
    mygpio_response_end(conn);

    // Get the value of GPIO number 5
    // It must be configured as an input GPIO in the configuration of myGPIOd.
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

    // Set the value of GPIO number 6 to high
    printf("Sending gpioset 6 1\n");
    if (mygpio_gpioset(conn, 6, MYGPIO_GPIO_VALUE_HIGH) == false) {
        printf("Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_connection_clear_error(conn);
    }
    mygpio_response_end(conn);

    // Close the connection
    printf("Closing connection\n");
    mygpio_connection_free(conn);

    return EXIT_SUCCESS;
}

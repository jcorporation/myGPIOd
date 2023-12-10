/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpioc/gpio.h"

#include "mygpio-common/util.h"
#include "mygpioc/util.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Lists the gpio configuration
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_gpiolist(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    (void)(argv);
    (void)option_index;
    verbose_printf("Sending gpiolist\n");
    if (mygpio_gpiolist(conn) == true) {
        struct t_mygpio_gpio_conf *gpio_conf;
        printf("Retrieving gpio config\n");
        while ((gpio_conf = mygpio_recv_gpio_conf(conn)) != NULL) {
            printf("GPIO %u, mode %u\n", gpio_conf->gpio, gpio_conf->mode);
            mygpio_free_gpio_conf(gpio_conf);
        }
        mygpio_response_end(conn);
        return EXIT_SUCCESS;
    }
    fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
    mygpio_response_end(conn);
    return EXIT_FAILURE;
}

/**
 * Gets the value of an input gpio
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_gpioget(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    unsigned gpio;
    if (mygpio_parse_uint(argv[option_index], &gpio, NULL, 0, GPIOS_MAX) == false) {
        fprintf(stderr, "Invalid gpio number\n");
        return EXIT_FAILURE;
    }
    verbose_printf("Sending gpioget 5\n");
    enum mygpio_gpio_value value = mygpio_gpioget(conn, gpio);
    if (value == MYGPIO_GPIO_VALUE_UNKNOWN) {
        fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_response_end(conn);
        return EXIT_FAILURE;
    }
    printf("Value: %u\n", value);
    mygpio_response_end(conn);
    return EXIT_SUCCESS;
}

/**
 * Sets an output gpio
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_gpioset(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    unsigned gpio;
    if (mygpio_parse_uint(argv[option_index], &gpio, NULL, 0, GPIOS_MAX) == false) {
        fprintf(stderr, "Invalid gpio number\n");
        return EXIT_FAILURE;
    }
    option_index++;
    unsigned value;
    if (mygpio_parse_uint(argv[option_index], &value, NULL, 0, 1) == false) {
        fprintf(stderr, "Invalid gpio value\n");
        return EXIT_FAILURE;
    }
    verbose_printf("Sending gpioset");
    if (mygpio_gpioset(conn, gpio, value) == false) {
        fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_response_end(conn);
        return EXIT_FAILURE;
    }
    mygpio_response_end(conn);
    return EXIT_SUCCESS;
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpioc/gpio.h"

#include "libmygpio/include/libmygpio/libmygpio.h"

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
    verbose_printf("Sending gpiolist");
    if (mygpio_gpiolist(conn) == true) {
        struct t_mygpio_gpio *gpio;
        verbose_printf("Retrieving configured gpios");
        while ((gpio = mygpio_recv_gpio_list(conn)) != NULL) {
            printf("GPIO %u, direction %s, value %s\n",
                mygpio_gpio_get_gpio(gpio),
                mygpio_gpio_lookup_direction(mygpio_gpio_get_direction(gpio)),
                mygpio_gpio_lookup_value(mygpio_gpio_get_value(gpio))
            );
            mygpio_free_gpio(gpio);
        }
        mygpio_response_end(conn);
        return EXIT_SUCCESS;
    }
    fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
    mygpio_response_end(conn);
    return EXIT_FAILURE;
}

/**
 * Gets the configuration of a gpio
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_gpioinfo(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    unsigned gpio_nr;
    if (mygpio_parse_uint(argv[option_index], &gpio_nr, NULL, 0, GPIOS_MAX) == false) {
        fprintf(stderr, "Invalid gpio number\n");
        return EXIT_FAILURE;
    }
    verbose_printf("Sending gpioinfo");
    if (mygpio_gpioinfo(conn, gpio_nr) == true) {
        verbose_printf("Retrieving gpio info");
        struct t_mygpio_gpio *gpio = mygpio_recv_gpio_info(conn);
        if (gpio == NULL) {
            return EXIT_FAILURE;
        }
        enum mygpio_gpio_direction direction = mygpio_gpio_get_direction(gpio);
        printf("GPIO: %u\n", mygpio_gpio_get_gpio(gpio));
        printf("Direction: %s\n", mygpio_gpio_lookup_direction(direction));
        printf("Value: %s\n", mygpio_gpio_lookup_value(mygpio_gpio_get_value(gpio)));
        if (direction == MYGPIO_GPIO_DIRECTION_IN) {
            printf("Active low: %s\n", mygpio_bool_to_str(mygpio_gpio_in_get_active_low(gpio)));
            printf("Bias: %s\n", mygpio_gpio_lookup_bias(mygpio_gpio_in_get_bias(gpio)));
            printf("Event request: %s\n", mygpio_gpio_lookup_event_request(mygpio_gpio_in_get_event_request(gpio)));
            printf("Is debounced: %s\n", mygpio_bool_to_str(mygpio_gpio_in_get_is_debounced(gpio)));
            printf("Debounce period: %d us\n", mygpio_gpio_in_get_debounce_period_us(gpio));
            printf("Event clock: %s\n", mygpio_gpio_lookup_event_clock(mygpio_gpio_in_get_event_clock(gpio)));
        }
        else if (direction == MYGPIO_GPIO_DIRECTION_OUT) {
            printf("Drive: %s\n", mygpio_gpio_lookup_drive(mygpio_gpio_out_get_drive(gpio)));
        }
        mygpio_free_gpio(gpio);
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
    verbose_printf("Sending gpioget");
    enum mygpio_gpio_value value = mygpio_gpioget(conn, gpio);
    if (value == MYGPIO_GPIO_VALUE_UNKNOWN) {
        fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_response_end(conn);
        return EXIT_FAILURE;
    }
    printf("Value: %s\n", mygpio_gpio_lookup_value(value));
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
    enum mygpio_gpio_value value;
    if ((value = mygpio_gpio_parse_value(argv[option_index])) == MYGPIO_GPIO_VALUE_UNKNOWN) {
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

/**
 * Toggles an output gpio
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_gpiotoggle(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    unsigned gpio;
    if (mygpio_parse_uint(argv[option_index], &gpio, NULL, 0, GPIOS_MAX) == false) {
        fprintf(stderr, "Invalid gpio number\n");
        return EXIT_FAILURE;
    }
    verbose_printf("Sending gpiotoggle");
    if (mygpio_gpiotoggle(conn, gpio) == false) {
        fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_response_end(conn);
        return EXIT_FAILURE;
    }
    mygpio_response_end(conn);
    return EXIT_SUCCESS;
}

/**
 * Toggles an output gpio at given timeout and interval.
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_gpioblink(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    unsigned gpio;
    if (mygpio_parse_uint(argv[option_index], &gpio, NULL, 0, GPIOS_MAX) == false) {
        fprintf(stderr, "Invalid gpio number\n");
        return EXIT_FAILURE;
    }
    int timeout;
    if (mygpio_parse_int(argv[option_index], &timeout, NULL, 0, 9999) == false) {
        fprintf(stderr, "Invalid timeout\n");
        return EXIT_FAILURE;
    }
    int interval;
    if (mygpio_parse_int(argv[option_index], &interval, NULL, 0, 9999) == false) {
        fprintf(stderr, "Invalid interval\n");
        return EXIT_FAILURE;
    }
    verbose_printf("Sending gpioblink");
    if (mygpio_gpioblink(conn, gpio, timeout, interval) == false) {
        fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_response_end(conn);
        return EXIT_FAILURE;
    }
    mygpio_response_end(conn);
    return EXIT_SUCCESS;
}

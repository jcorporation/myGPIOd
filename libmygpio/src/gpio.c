/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "libmygpio/include/libmygpio/gpio.h"
#include "libmygpio/include/libmygpio/protocol.h"
#include "libmygpio/src/pair.h"
#include "libmygpio/src/protocol.h"
#include "mygpio-common/util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Receives the value of an input or output gpio
 * @param connection connection struct
 * @param gpio gpio number (0-99)
 * @return value of the gpio or MYGPIO_GPIO_VALUE_UNKNOWN on error
 */
enum mygpio_gpio_value mygpio_gpioget(struct t_mygpio_connection *connection, unsigned gpio) {
    unsigned value;
    struct t_mygpio_pair *pair;
    if (gpio > GPIOS_MAX) {
        return MYGPIO_GPIO_VALUE_UNKNOWN;
    }
    if (libmygpio_send_line(connection, "gpioget %u", gpio) == false ||
        libmygpio_recv_response_status(connection) == false ||
        (pair = mygpio_recv_pair(connection)) == NULL ||
        strcmp(pair->name, "value") != 0 ||
        mygpio_parse_uint(pair->value, &value, NULL, 0, 1) == false)
    {
        return MYGPIO_GPIO_VALUE_UNKNOWN;
    }
    return value;
}

/**
 * Sets the value of an output gpio
 * @param connection connection struct
 * @param gpio gpio number (0-99)
 * @param value gpio value
 * @return true on success, else false
 */
bool mygpio_gpioset(struct t_mygpio_connection *connection, unsigned gpio, enum mygpio_gpio_value value) {
    if (gpio > GPIOS_MAX) {
        return false;
    }
    return libmygpio_send_line(connection, "gpioset %u %u", gpio, value) &&
        libmygpio_recv_response_status(connection) &&
        mygpio_response_end(connection);
}

/**
 * Toggles the value of a configured output GPIO.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param gpio GPIO number
 * @return true on success, else false.
 */
bool mygpio_gpiotoggle(struct t_mygpio_connection *connection, unsigned gpio) {
    if (gpio > GPIOS_MAX) {
        return false;
    }
    return libmygpio_send_line(connection, "gpiotoggle %u", gpio) &&
        libmygpio_recv_response_status(connection) &&
        mygpio_response_end(connection);
}

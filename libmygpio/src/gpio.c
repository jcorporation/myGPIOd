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

// private definitions

static enum mygpio_gpio_mode parse_gpio_mode(const char *str);

// public functions

/**
 * Send the gpiolist command and receives the status
 * @param connection connection struct
 * @return true on success, else false
 */
bool mygpio_gpiolist(struct t_mygpio_connection *connection) {
    return libmygpio_send_line(connection, "gpiolist") &&
        libmygpio_recv_response_status(connection);
}

/**
 * Receives the response for the gpiolist command
 * @param connection connection struct
 * @return gpio config struct or NULL on error or list end
 */
struct t_mygpio_gpio_conf *mygpio_recv_gpio_conf(struct t_mygpio_connection *connection) {
    unsigned gpio;
    enum mygpio_gpio_mode mode;

    struct t_mygpio_pair *pair = mygpio_recv_pair(connection);
    if (pair == NULL ||
        strcmp(pair->name, "gpio") != 0 ||
        mygpio_parse_uint(pair->value, &gpio, NULL, 0, GPIOS_MAX) == false)
    {
        if (pair != NULL) {
            mygpio_free_pair(pair);
        }
        return NULL;
    }
    mygpio_free_pair(pair);

    pair = mygpio_recv_pair(connection);
    if (pair == NULL ||
        strcmp(pair->name, "mode") != 0 ||
        (mode = parse_gpio_mode(pair->value)) == MYGPIO_GPIO_MODE_UNKNOWN)
    {
        if (pair != NULL) {
            mygpio_free_pair(pair);
        }
        return NULL;
    }
    mygpio_free_pair(pair);

    struct t_mygpio_gpio_conf *gpio_conf = malloc(sizeof(struct t_mygpio_gpio_conf));
    assert(gpio_conf);
    gpio_conf->gpio = gpio;
    gpio_conf->mode = mode;
    return gpio_conf;
}

/**
 * Frees the gpio config struct
 * @param gpio_conf 
 */
void mygpio_free_gpio_conf(struct t_mygpio_gpio_conf *gpio_conf) {
    free(gpio_conf);
}

/**
 * Receives the value of an input gpio
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
    char command[11];
    snprintf(command, 11, "gpioget %u", gpio);
    if (libmygpio_send_line(connection, command) != true ||
        libmygpio_recv_response_status(connection) != true ||
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
 * @return value of the gpio or MYGPIO_GPIO_VALUE_UNKNOWN on error
 */
bool mygpio_gpioset(struct t_mygpio_connection *connection, unsigned gpio, enum mygpio_gpio_value value) {
    if (gpio > 99) {
        return false;
    }
    char command[14];
    snprintf(command, 14, "gpioset %u %u", gpio, value);
    return libmygpio_send_line(connection, command) &&
        libmygpio_recv_response_status(connection) &&
        mygpio_response_end(connection);
}

// private functions

/**
 * Parses the gpio mode
 * @param str string to parse
 * @return mode of the gpio
 */
static enum mygpio_gpio_mode parse_gpio_mode(const char *str) {
    if (strcmp(str, "in") == 0) {
        return MYGPIO_GPIO_MODE_IN;
    }
    if (strcmp(str, "out") == 0) {
        return MYGPIO_GPIO_MODE_OUT;
    }
    return MYGPIO_GPIO_MODE_UNKNOWN;
}

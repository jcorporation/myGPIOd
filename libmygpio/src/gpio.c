/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "libmygpio/include/libmygpio/gpio.h"
#include "libmygpio/include/libmygpio/protocol.h"
#include "libmygpio/src/gpio.h"
#include "libmygpio/src/pair.h"
#include "libmygpio/src/protocol.h"
#include "mygpio-common/util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Lookups the name for the gpio mode.
 * @param mode the gpio mode.
 * @return gpio mode name
 */
const char *mygpio_gpio_lookup_mode(enum mygpio_gpio_mode mode) {
    switch(mode) {
        case MYGPIO_GPIO_MODE_IN:
            return "in";
        case MYGPIO_GPIO_MODE_OUT:
            return "out";
        case MYGPIO_GPIO_MODE_UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

/**
 * Parses a string to the gpio mode.
 * @param str string to parse
 * @return mode of the gpio
 */
enum mygpio_gpio_mode parse_gpio_mode(const char *str) {
    if (strcmp(str, "in") == 0) {
        return MYGPIO_GPIO_MODE_IN;
    }
    if (strcmp(str, "out") == 0) {
        return MYGPIO_GPIO_MODE_OUT;
    }
    return MYGPIO_GPIO_MODE_UNKNOWN;
}


/**
 * Lookups the name for the gpio value.
 * @param value the gpio value.
 * @return gpio value name
 */
const char *mygpio_gpio_lookup_value(enum mygpio_gpio_value value) {
    switch(value) {
        case MYGPIO_GPIO_VALUE_HIGH:
            return "high";
        case MYGPIO_GPIO_VALUE_LOW:
            return "low";
        case MYGPIO_GPIO_VALUE_UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

/**
 * Parses a string to a gpio value.
 * @param str string to parse
 * @return gpio value or GPIO_VALUE_LOW on error
 */
enum mygpio_gpio_value mygpio_parse_gpio_value(const char *str) {
    if (strcasecmp(str, "active") == 0 ||
        strcasecmp(str, "high") == 0 ||
        strcasecmp(str, "on") == 0 ||
        strcmp(str, "1") == 0)
    {
        return MYGPIO_GPIO_VALUE_HIGH;
    }
    if (strcasecmp(str, "inactive") == 0 ||
        strcasecmp(str, "low") == 0 ||
        strcasecmp(str, "off") == 0 ||
        strcmp(str, "0") == 0)
    {
        return MYGPIO_GPIO_VALUE_LOW;
    }
    return MYGPIO_GPIO_VALUE_UNKNOWN;
}

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
 * Returns the GPIO number from struct t_mygpio_gpio_conf.
 * @param gpio_conf Pointer to struct t_mygpio_gpio_conf.
 * @return GPIO number. 
 */
unsigned mygpio_gpio_conf_get_gpio(struct t_mygpio_gpio_conf *gpio_conf) {
    return gpio_conf->gpio;
}

/**
 * Returns the GPIO mode from struct t_mygpio_gpio_conf.
 * @param gpio_conf Pointer to struct t_mygpio_gpio_conf.
 * @return GPIO mode, one of enum mygpio_gpio_mode.
 */
enum mygpio_gpio_mode mygpio_gpio_conf_get_mode(struct t_mygpio_gpio_conf *gpio_conf) {
    return gpio_conf->mode;
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
    if (libmygpio_send_line(connection, command) == false ||
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
    if (gpio > 99) {
        return false;
    }
    char command[14];
    snprintf(command, 14, "gpioset %u %u", gpio, value);
    return libmygpio_send_line(connection, command) &&
        libmygpio_recv_response_status(connection) &&
        mygpio_response_end(connection);
}

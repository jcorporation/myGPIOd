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
enum mygpio_gpio_mode mygpio_gpio_parse_mode(const char *str) {
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
enum mygpio_gpio_value mygpio_gpio_parse_value(const char *str) {
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
 * Lookups the name for the gpio bias.
 * @param bias the gpio bias.
 * @return gpio bias name
 */
const char *mygpio_gpio_lookup_bias(enum mygpio_gpio_bias bias) {
    switch(bias) {
        case MYGPIO_BIAS_AS_IS:
            return "as-is";
        case MYGPIO_BIAS_DISABLED:
            return "disable";
        case MYGPIO_BIAS_PULL_DOWN:
            return "pull-down";
        case MYGPIO_BIAS_PULL_UP:
            return "pull-up";
        case MYGPIO_EVENT_REQUEST_UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

/**
 * Parses a string to a gpio bias.
 * @param str string to parse
 * @return gpio bias or GPIO_BIAS_UNKNOWN on error
 */
enum mygpio_gpio_bias mygpio_gpio_parse_bias(const char *str) {
    if (strcasecmp(str, "as-is") == 0) {
        return MYGPIO_BIAS_AS_IS;
    }
    if (strcasecmp(str, "disable") == 0) {
        return MYGPIO_BIAS_DISABLED;
    }
    if (strcasecmp(str, "pull-down") == 0) {
        return MYGPIO_BIAS_PULL_DOWN;
    }
    if (strcasecmp(str, "pull-up") == 0) {
        return MYGPIO_BIAS_PULL_UP;
    }
    return MYGPIO_BIAS_UNKNOWN;
}

/**
 * Lookups the name for an event request.
 * @param value the gpio event request.
 * @return gpio value name
 */
const char *mygpio_gpio_lookup_event_request(enum mygpio_event_request event_request) {
    switch(event_request) {
        case MYGPIO_EVENT_REQUEST_FALLING:
            return "monotonic";
        case MYGPIO_EVENT_REQUEST_RISING:
            return "realtime";
        case MYGPIO_EVENT_REQUEST_BOTH:
            return "hte";
        case MYGPIO_EVENT_REQUEST_UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

/**
 * Parses a string to an event request.
 * @param str string to parse
 * @return gpio event request or GPIO_EVENT_REQUEST_UNKNOWN on error
 */
enum mygpio_event_request mygpio_gpio_parse_event_request(const char *str) {
    if (strcasecmp(str, "falling") == 0) {
        return MYGPIO_EVENT_REQUEST_FALLING;
    }
    if (strcasecmp(str, "rising") == 0) {
        return MYGPIO_EVENT_REQUEST_RISING;
    }
    if (strcasecmp(str, "both") == 0) {
        return MYGPIO_EVENT_REQUEST_BOTH;
    }
    return MYGPIO_EVENT_REQUEST_UNKNOWN;
}

/**
 * Lookups the name for the gpio event clock.
 * @param clock the gpio clock.
 * @return gpio clock name
 */
const char *mygpio_gpio_lookup_event_clock(enum mygpio_event_clock clock) {
    switch(clock) {
        case MYGPIO_EVENT_CLOCK_MONOTONIC:
            return "monotonic";
        case MYGPIO_EVENT_CLOCK_REALTIME:
            return "realtime";
        case MYGPIO_EVENT_CLOCK_HTE:
            return "hte";
        case MYGPIO_EVENT_CLOCK_UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

/**
 * Parses a string to a gpio event clock.
 * @param str string to parse
 * @return gpio event clock or MYGPIO_EVENT_CLOCK_UNKNOWN on error
 */
enum mygpio_event_clock mygpio_gpio_parse_event_clock(const char *str) {
    if (strcasecmp(str, "monotonic") == 0) {
        return MYGPIO_EVENT_CLOCK_MONOTONIC;
    }
    if (strcasecmp(str, "realtime") == 0) {
        return MYGPIO_EVENT_CLOCK_REALTIME;
    }
    if (strcasecmp(str, "hte") == 0) {
        return MYGPIO_EVENT_CLOCK_HTE;
    }
    return MYGPIO_EVENT_CLOCK_UNKNOWN;
}

/**
 * Lookups the name for the gpio drive setting.
 * @param drive the gpio drive.
 * @return gpio drive name
 */
const char *mygpio_gpio_lookup_drive(enum mygpio_drive drive) {
    switch(drive) {
        case MYGPIO_DRIVE_PUSH_PULL:
            return "push-pull";
        case MYGPIO_DRIVE_OPEN_DRAIN:
            return "open-drain";
        case MYGPIO_DRIVE_OPEN_SOURCE:
            return "open-source";
        case MYGPIO_DRIVE_UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

/**
 * Parses a string to a gpio drive.
 * @param str string to parse
 * @return gpio bias or MYGPIO_DRIVE_UNKNOWN on error
 */
enum mygpio_drive mygpio_gpio_parse_drive(const char *str) {
    if (strcasecmp(str, "push-pull") == 0) {
        return MYGPIO_DRIVE_PUSH_PULL;
    }
    if (strcasecmp(str, "open-drain") == 0) {
        return MYGPIO_DRIVE_OPEN_DRAIN;
    }
    if (strcasecmp(str, "open-source") == 0) {
        return MYGPIO_DRIVE_OPEN_SOURCE;
    }
    return MYGPIO_DRIVE_UNKNOWN;
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

/**
 * Returns the GPIO number from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO number. 
 */
unsigned mygpio_gpio_get_gpio(struct t_mygpio_gpio *gpio) {
    return gpio->gpio;
}

/**
 * Returns the GPIO mode from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO mode, one of enum mygpio_gpio_mode.
 */
enum mygpio_gpio_mode mygpio_gpio_get_mode(struct t_mygpio_gpio *gpio) {
    return gpio->mode;
}

/**
 * Returns the GPIO value from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO mode, one of enum mygpio_gpio_mode.
 */
enum mygpio_gpio_value mygpio_gpio_get_value(struct t_mygpio_gpio *gpio) {
    return gpio->value;
}

/**
 * Returns the GPIO active_low from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO is set to active_low?
 */
bool mygpio_gpio_in_get_active_low(struct t_mygpio_gpio *gpio) {
    assert(gpio->in);
    return gpio->in->active_low;
}

/**
 * Returns the GPIO bias from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO bias, one of enum mygpio_gpio_bias.
 */
enum mygpio_gpio_bias mygpio_gpio_in_get_bias(struct t_mygpio_gpio *gpio) {
    assert(gpio->in);
    return gpio->in->bias;
}

/**
 * Returns the requested events from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return requested GPIO events, one of enum event_request.
 */
enum mygpio_event_request mygpio_gpio_in_get_event_request(struct t_mygpio_gpio *gpio) {
    assert(gpio->in);
    return gpio->in->event_request;
}

/**
 * Returns if the GPIO is debounced.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO debounced?
 */
bool mygpio_gpio_in_get_is_debounced(struct t_mygpio_gpio *gpio) {
    assert(gpio->in);
    return gpio->in->is_debounced;
}

/**
 * Returns the GPIO debounce period from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO debounce period in nanoseconds.
 */
int mygpio_gpio_in_get_debounce_period(struct t_mygpio_gpio *gpio) {
    assert(gpio->in);
    return gpio->in->debounce_period;
}

/**
 * Returns the GPIO event clock from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO event clock, one of enum mygpio_event_clock.
 */
enum mygpio_event_clock mygpio_gpio_in_get_event_clock(struct t_mygpio_gpio *gpio) {
    assert(gpio->in);
    return gpio->in->event_clock;
}

/**
 * Returns the GPIO drive setting from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO drive setting, one of enum mygpio_drive.
 */
enum mygpio_drive mygpio_gpio_out_get_drive(struct t_mygpio_gpio *gpio) {
    assert(gpio->out);
    return gpio->out->drive;
}

/**
 * Creates a new gpio struct
 * @return struct t_mygpio_gpio* 
 */
struct t_mygpio_gpio *mygpio_gpio_new(enum mygpio_gpio_mode mode) {
    struct t_mygpio_gpio *gpio = malloc(sizeof(struct t_mygpio_gpio));
    assert(gpio);
    gpio->in = NULL;
    gpio->out = NULL;
    if (mode == MYGPIO_GPIO_MODE_IN) {
        gpio->in = malloc(sizeof(struct t_mygpio_in));
        assert(gpio->in);
    }
    else if (mode == MYGPIO_GPIO_MODE_OUT) {
        gpio->out = malloc(sizeof(struct t_mygpio_out));
        assert(gpio->out);
    }
    return gpio;
}

/**
 * Frees the gpio struct
 * @param gpio 
 */
void mygpio_free_gpio(struct t_mygpio_gpio *gpio) {
    if (gpio->in != NULL) {
        free(gpio->in);
    }
    if (gpio->out != NULL) {
        free(gpio->out);
    }
    free(gpio);
}

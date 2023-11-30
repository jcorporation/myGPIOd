/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "util.h"

#include "config.h"
#include "log.h"

#include <ctype.h>
#include <errno.h>
#include <gpiod.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/**
 * Parses a string to a gpio value.
 * @param str string to parse
 * @return gpio value or GPIO_VALUE_LOW on error
 */
int parse_gpio_value(const char *str) {
    if (strcasecmp(str, "high") == 0) {
        return GPIO_VALUE_HIGH;
    }
    if (strcasecmp(str, "low") == 0) {
        return GPIO_VALUE_LOW;
    }
    return GPIO_VALUE_LOW;
}

/**
 * Lookups the name for a gpio value
 * @param value value
 * @return name
 */
const char *lookup_gpio_value(int value) {
    switch(value) {
        case GPIO_VALUE_HIGH:
            return "high";
        case GPIO_VALUE_LOW:
            return "low";
    }
    MYGPIOD_LOG_WARN("Could not lookup gpio value");
    return "";
}

/**
 * Parses a string to a boolean value.
 * Returns false if string is not true.
 * @param str string to parse
 * @return parsed value
 */
bool parse_bool(const char *str) {
    if (strcasecmp(str, "true") == 0) {
        return true;
    }
    if (strcasecmp(str, "false") == 0) {
        return false;
    }
    MYGPIOD_LOG_WARN("Could not parse bool, setting default");
    return false;
}

/**
 * Prints a bool value as string
 * @param v the bool value
 * @return string
 */
const char *bool_to_str(bool v) {
    return v == true
        ? "true"
        : "false";
}

/**
 * Parses the bias flags
 * @param option string to parse
 * @return parsed bias flag or as-is on error
 */
int parse_bias(const char *str) {
    if (strcasecmp(str, "pull-down") == 0) {
        return GPIOD_LINE_BIAS_PULL_DOWN;
    }
    if (strcasecmp(str, "pull-up") == 0) {
        return GPIOD_LINE_BIAS_PULL_UP;
    }
    if (strcasecmp(str, "disable") == 0) {
        return GPIOD_LINE_BIAS_DISABLE;
    }
    if (strcasecmp(str, "as-is") == 0) {
        return GPIOD_LINE_BIAS_AS_IS;
    }
    // Bias set to as-is
    MYGPIOD_LOG_WARN("Could not parse bias, setting default");
    return 0;
}

/**
 * Lookups the bias name
 * @param bias the bias flag
 * @return name
 */
const char *lookup_bias(int bias) {
    switch(bias) {
        case GPIOD_LINE_BIAS_AS_IS:
            return "as-is";
        case GPIOD_LINE_BIAS_PULL_DOWN:
            return "pull-down";
        case GPIOD_LINE_BIAS_PULL_UP:
            return "pull-up";
        case GPIOD_LINE_BIAS_DISABLE:
            return "disable";
    }
    MYGPIOD_LOG_WARN("Could not lookup bias");
    return "";
}

/**
 * Parses the request event setting for the chip.
 * @param str string to parse
 * @return GPIOD_LINE_REQUEST_EVENT enum or GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE on error
 */
int parse_event_request(const char *str) {
    if (strcasecmp(str, "falling") == 0) {
        return GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE;
    }
    if (strcasecmp(str, "rising") == 0) {
        return GPIOD_LINE_REQUEST_EVENT_RISING_EDGE;
    }
    if (strcasecmp(str, "both") == 0) {
        return GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES;
    }
    MYGPIOD_LOG_WARN("Could not parse event request, setting default");
    return GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE;
}

/**
 * Lookups the string for the event request
 * @param event events to request
 * @return the name of the event request
 */
const char *lookup_event_request(int event) {
    switch(event) {
        case GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE:
            return "falling";
        case GPIOD_LINE_REQUEST_EVENT_RISING_EDGE:
            return "rising";
        case GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES:
            return "both";
    }
    MYGPIOD_LOG_WARN("Could not lookup event request");
    return "";
}

/**
 * Lookups the string for a gpio event
 * @param event the event
 * @return name of the event
 */
const char *lookup_event(int event) {
    switch(event) {
        case GPIOD_LINE_EVENT_FALLING_EDGE:
            return "falling";
        case GPIOD_LINE_EVENT_RISING_EDGE:
            return "rising";
    }
    MYGPIOD_LOG_WARN("Could not lookup event");
    return "";
}

/**
 * Parses the start of a string to an unsigned value and checks it against min and max.
 * @param str string to parse
 * @param result pointer for the result
 * @param rest pointer to first none numeric char
 * @param min minimum value (including)
 * @param max maximum value (including)
 * @return bool true on success, else false
 */
bool parse_uint(char *str, unsigned *result, char **rest, unsigned min, unsigned max) {
    errno = 0;
    uintmax_t v = strtoumax(str, rest, 10);
    if (errno == 0 && v >= min && v <= max) {
        *result = (unsigned)v;
        return true;
    }
    MYGPIOD_LOG_WARN("Invalid value");
    return false;
}

/**
 * Parses the start of a string to an integer value and checks it against min and max.
 * @param str string to parse
 * @param result pointer for the result
 * @param rest pointer to first none numeric char
 * @param min minimum value (including)
 * @param max maximum value (including)
 * @return bool true on success, else false
 */
bool parse_int(char *str, int *result, char **rest, unsigned min, unsigned max) {
    errno = 0;
    intmax_t v = strtoimax(str, rest, 10);
    if (errno == 0 && v >= min && v <= max) {
        *result = (int)v;
        return true;
    }
    MYGPIOD_LOG_WARN("Invalid value");
    return false;
}

/**
 * Removes whitespace characters from end
 * @param line string to chomp
 * @return chomped string
 */
char *chomp(char *line, size_t len) {
    size_t i = len - 1;
    while (i > 0 && isspace(line[i])) {
        i--;
    }
    line[i + 1] = '\0';
    return line;
}

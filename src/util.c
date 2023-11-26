/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "util.h"

#include "log.h"

#include <errno.h>
#include <gpiod.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/signalfd.h>

/**
 * Creates a signalfd to exit on SIGTERM and SIGINT
 * @return the created signal fd
 */
int make_signalfd(void) {
    sigset_t sigmask;
    int sigfd, rv;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGINT);

    errno = 0;
    rv = sigprocmask(SIG_BLOCK, &sigmask, NULL);
    if (rv < 0) {
        MYGPIOD_LOG_ERROR("Error masking signals: \"%s\"", strerror(errno));
        return -1;
    }

    errno = 0;
    sigfd = signalfd(-1, &sigmask, 0);
    if (sigfd < 0) {
        MYGPIOD_LOG_ERROR("Error creating signalfd: \"%s\"", strerror(errno));
        return -1;
    }

    return sigfd;
}

/**
 * Checks if the value is in the array
 * @param value value to match
 * @param array array to check
 * @param len length of array
 * @return true if value is in array, else false
 */
bool value_in_array(unsigned value, unsigned *array, size_t len) {
    for(size_t i = 0; i < len; i++) {
        if(array[i] == value)
            return true;
    }
    return false;
}

/**
 * Lookups the name for a gpio value
 * @param value value
 * @return name
 */
const char *lookup_gpio_value(enum gpio_values value) {
    switch(value) {
        case GPIO_VALUE_UNSET:
            return "unset";
        case GPIO_VALUE_HIGH:
            return "high";
        case GPIO_VALUE_LOW:
            return "low";
    }
    MYGPIOD_LOG_WARN("Could not lookup gpio value");
    return "";
}

/**
 * Parses the chip value
 * @param str string to parse
 * @param config pointer to config
 * @return true on success, else false
 */
bool parse_chip(char *str, struct t_config *config) {
    unsigned chip;
    if (parse_uint(str, &chip, NULL, 0, 9) == false) {
        return false;
    }
    if (snprintf(config->chip, 2, "%u", chip) == 1) {
        return true;
    }
    MYGPIOD_LOG_WARN("Invalid value");
    return false;
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
        return GPIOD_CTXLESS_FLAG_BIAS_PULL_DOWN;
    }
    if (strcasecmp(str, "pull-up") == 0) {
        return GPIOD_CTXLESS_FLAG_BIAS_PULL_UP;
    }
    if (strcasecmp(str, "disable") == 0) {
        return GPIOD_CTXLESS_FLAG_BIAS_DISABLE;
    }
    if (strcasecmp(str, "as-is") == 0) {
        return 0;
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
        case 0:
            return "as-is";
        case GPIOD_CTXLESS_FLAG_BIAS_PULL_DOWN:
            return "pull-down";
        case GPIOD_CTXLESS_FLAG_BIAS_PULL_UP:
            return "pull-up";
        case GPIOD_CTXLESS_FLAG_BIAS_DISABLE:
            return "disable";
    }
    MYGPIOD_LOG_WARN("Could not lookup bias");
    return "";
}

/**
 * Parses the edge setting.
 * @param str string to parse
 * @return GPIOD_CTXLESS_EVENT enum or GPIOD_CTXLESS_EVENT_FALLING_EDGE on error
 */
int parse_edge(const char *str) {
    if (strcasecmp(str, "falling") == 0) {
        return GPIOD_CTXLESS_EVENT_FALLING_EDGE;
    }
    if (strcasecmp(str, "rising") == 0) {
        return GPIOD_CTXLESS_EVENT_RISING_EDGE;
    }
    if (strcasecmp(str, "both") == 0) {
        return GPIOD_CTXLESS_EVENT_BOTH_EDGES;
    }
    MYGPIOD_LOG_WARN("Could not parse edge, setting default");
    return GPIOD_CTXLESS_EVENT_FALLING_EDGE;
}

/**
 * Lookups the string for CTXLESS_EVENT_*
 * @param event the CTXLESS_EVENT enum
 * @return name
 */
const char *lookup_ctxless_event(int event) {
    switch(event) {
        case GPIOD_CTXLESS_EVENT_FALLING_EDGE:
            return "falling";
        case GPIOD_CTXLESS_EVENT_RISING_EDGE:
            return "rising";
        case GPIOD_CTXLESS_EVENT_BOTH_EDGES:
            return "both";
    }
    MYGPIOD_LOG_WARN("Could not lookup ctxless event");
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

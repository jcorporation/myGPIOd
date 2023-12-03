/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "src/lib/util.h"

#include "dist/sds/sds.h"
#include "src/lib/config.h"
#include "src/lib/log.h"

#include <ctype.h>
#include <errno.h>
#include <gpiod.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


/**
 * Closes an open file descriptor.
 * Checks it if is open and sets it to -1.
 * @param fd 
 */
void close_fd(int *fd) {
    if (*fd > -1) {
        close(*fd);
        *fd = -1;
    }
}

/**
 * Getline function that trims whitespace characters
 * @param s an already allocated sds string
 * @param fp a file descriptor to read from
 * @param max max line length to read
 * @return Number of bytes read or -1 on eof
 */
int sds_getline(sds *s, FILE *fp, size_t max) {
    sdsclear(*s);
    for (size_t i = 0; i < max; i++) {
        int c = fgetc(fp);
        if (c == EOF ||
            c == '\n')
        {
            sdstrim(*s, "\r \t");
            return c == EOF ? -1 : (int)sdslen(*s);
        }
        *s = sds_catchar(*s, (char)c);
    }
    MYGPIOD_LOG_ERROR("Line is too long, max length is %lu", (unsigned long)max);
    sdstrim(*s, "\r \t");
    return (int)sdslen(*s);
}

/**
 * Appends a char to sds string s, this is faster than using sdscatfmt
 * @param s sds string
 * @param c char to append
 * @return modified sds string
 */
sds sds_catchar(sds s, const char c) {
    // Make sure there is always space for at least 1 char.
    if (sdsavail(s) == 0) {
        s = sdsMakeRoomFor(s, 1);
    }
    size_t i = sdslen(s);
    s[i++] = c;
    sdsinclen(s, 1);
    // Add null-term
    s[i] = '\0';
    return s;
}

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

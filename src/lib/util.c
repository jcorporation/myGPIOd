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
 * Sets errno to EINVAL on error.
 * @param str string to parse
 * @return gpio value or GPIO_VALUE_LOW on error
 */
enum gpiod_line_value parse_gpio_value(const char *str) {
    if (strcasecmp(str, "active") == 0 ||
        strcasecmp(str, "high") == 0 ||
        strcmp(str, "1") == 0)
    {
        return GPIOD_LINE_VALUE_ACTIVE;
    }
    if (strcasecmp(str, "inactive") == 0 ||
        strcasecmp(str, "low") == 0 ||
        strcmp(str, "0") == 0)
    {
        return GPIOD_LINE_VALUE_INACTIVE;
    }
    if (strcasecmp(str, "error") == 0) {
        return GPIOD_LINE_VALUE_ERROR;
    }
    errno = EINVAL;
    MYGPIOD_LOG_WARN("Could not parse gpio value, setting error");
    return GPIOD_LINE_VALUE_ERROR;
}

/**
 * Lookups the string for a gpio value
 * @param value value
 * @return value string
 */
const char *lookup_gpio_value(enum gpiod_line_value value) {
    switch(value) {
        case GPIOD_LINE_VALUE_ERROR:
            return "error";
        case GPIOD_LINE_VALUE_INACTIVE:
            return "inactive";
        case GPIOD_LINE_VALUE_ACTIVE:
            return "active";
    }
    MYGPIOD_LOG_WARN("Could not lookup gpio value, setting error");
    return "error";
}

/**
 * Parses a string to a gpio drive setting.
 * @param str string to parse
 * @return gpio value or GPIO_VALUE_LOW on error
 */
enum gpiod_line_drive parse_drive(const char *str) {
    if (strcasecmp(str, "push-pull") == 0) {
        return GPIOD_LINE_DRIVE_PUSH_PULL;
    }
    if (strcasecmp(str, "open-drain") == 0) {
        return GPIOD_LINE_DRIVE_OPEN_DRAIN;
    }
    if (strcasecmp(str, "open-source") == 0) {
        return GPIOD_LINE_DRIVE_OPEN_SOURCE;
    }
    MYGPIOD_LOG_WARN("Could not parse gpio value, setting push-pull");
    return GPIOD_LINE_DRIVE_PUSH_PULL;
}

/**
 * Lookups the string for a gpio drive
 * @param value value
 * @return value string
 */
const char *lookup_drive(enum gpiod_line_drive value) {
    switch(value) {
        case GPIOD_LINE_DRIVE_PUSH_PULL:
            return "push-pull";
        case GPIOD_LINE_DRIVE_OPEN_DRAIN:
            return "open-drain";
        case GPIOD_LINE_DRIVE_OPEN_SOURCE:
            return "open-source";
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
enum gpiod_line_bias parse_bias(const char *str) {
    if (strcasecmp(str, "pull-down") == 0) {
        return GPIOD_LINE_BIAS_PULL_DOWN;
    }
    if (strcasecmp(str, "pull-up") == 0) {
        return GPIOD_LINE_BIAS_PULL_UP;
    }
    if (strcasecmp(str, "disable") == 0) {
        return GPIOD_LINE_BIAS_DISABLED;
    }
    if (strcasecmp(str, "as-is") == 0) {
        return GPIOD_LINE_BIAS_AS_IS;
    }
    if (strcasecmp(str, "unknown") == 0) {
        return GPIOD_LINE_BIAS_UNKNOWN;
    }
    MYGPIOD_LOG_WARN("Could not parse bias, setting unknown");
    return GPIOD_LINE_BIAS_UNKNOWN;
}

/**
 * Lookups the bias name
 * @param bias the bias flag
 * @return name
 */
const char *lookup_bias(enum gpiod_line_bias bias) {
    switch(bias) {
        case GPIOD_LINE_BIAS_AS_IS:
            return "as-is";
        case GPIOD_LINE_BIAS_UNKNOWN:
            return "unknown";
        case GPIOD_LINE_BIAS_PULL_DOWN:
            return "pull-down";
        case GPIOD_LINE_BIAS_PULL_UP:
            return "pull-up";
        case GPIOD_LINE_BIAS_DISABLED:
            return "disable";
    }
    MYGPIOD_LOG_WARN("Could not lookup bias, setting unknown");
    return "unknown";
}

/**
 * Parses the clock setting.
 * @param str string to parse
 * @return enum gpiod_line_clock
 */
enum gpiod_line_clock parse_event_clock(const char *str) {
    if (strcasecmp(str, "realtime") == 0) {
        return GPIOD_LINE_CLOCK_REALTIME;
    }
    if (strcasecmp(str, "hte") == 0) {
        return GPIOD_LINE_CLOCK_HTE;
    }
    if (strcmp(str, "monotonic") == 0) {
        return GPIOD_LINE_CLOCK_MONOTONIC;
    }
    MYGPIOD_LOG_WARN("Could not parse event request, setting monotonic");
    return GPIOD_LINE_CLOCK_MONOTONIC;
}

/**
 * Parses the request event setting.
 * @param str string to parse
 * @return gpiod_line_edge enum
 */
enum gpiod_line_edge parse_event_request(const char *str) {
    if (strcasecmp(str, "none") == 0) {
        return GPIOD_LINE_EDGE_NONE;
    }
    if (strcasecmp(str, "falling") == 0) {
        return GPIOD_LINE_EDGE_FALLING;
    }
    if (strcasecmp(str, "rising") == 0) {
        return GPIOD_LINE_EDGE_RISING;
    }
    if (strcasecmp(str, "both") == 0) {
        return GPIOD_LINE_EDGE_BOTH;
    }
    MYGPIOD_LOG_WARN("Could not parse event request, setting none");
    return GPIOD_LINE_EDGE_NONE;
}

/**
 * Lookups the string for the event request
 * @param event events to request
 * @return the name of the event request
 */
const char *lookup_event_request(enum gpiod_line_edge event) {
    switch(event) {
        case GPIOD_LINE_EDGE_NONE:
            return "none";
        case GPIOD_LINE_EDGE_RISING:
            return "falling";
        case GPIOD_LINE_EDGE_FALLING:
            return "rising";
        case GPIOD_LINE_EDGE_BOTH:
            return "both";
    }
    MYGPIOD_LOG_WARN("Could not lookup event request, setting none");
    return "none";
}

/**
 * Lookups the string for the event type
 * @param event event type
 * @return the name of the event type
 */
const char *lookup_event_type(enum gpiod_edge_event_type event) {
    switch(event) {
        case GPIOD_EDGE_EVENT_FALLING_EDGE:
            return "falling";
        case GPIOD_EDGE_EVENT_RISING_EDGE:
            return "rising";
    }
    MYGPIOD_LOG_WARN("Could not lookup event type");
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
 * Parses the start of a string to an unsigned long value and checks it against min and max.
 * @param str string to parse
 * @param result pointer for the result
 * @param rest pointer to first none numeric char
 * @param min minimum value (including)
 * @param max maximum value (including)
 * @return bool true on success, else false
 */
bool parse_ulong(char *str, unsigned long *result, char **rest, unsigned min, unsigned max) {
    errno = 0;
    uintmax_t v = strtoumax(str, rest, 10);
    if (errno == 0 && v >= min && v <= max) {
        *result = (unsigned long)v;
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

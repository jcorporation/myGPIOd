/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/lib/util.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"

#include <ctype.h>
#include <errno.h>
#include <gpiod.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Returns a timestamp with nanoseconcs precision
 * @param event_clock clock to use
 * @return timestamp
 */
uint64_t get_timestamp_ns(enum gpiod_line_clock event_clock) {
    struct timespec ts;
    switch (event_clock) {
        case GPIOD_LINE_CLOCK_HTE:
            //TODO: howto handle this?
        case GPIOD_LINE_CLOCK_REALTIME:
            clock_gettime(CLOCK_REALTIME, &ts);
            break;
        case GPIOD_LINE_CLOCK_MONOTONIC:
            clock_gettime(CLOCK_MONOTONIC, &ts);
            break;
    }
    return (uint64_t)(ts.tv_sec * 1000000000 + ts.tv_nsec);
}

/**
 * Getline function that trims whitespace characters.
 * @param s an already allocated sds string
 * @param fp a file descriptor to read from
 * @param max max line length to read
 * @param nread read bytes
 * @return Pointer to s
 */
sds sds_getline(sds s, FILE *fp, size_t max, int *nread) {
    sdsclear(s);
    s = sdsMakeRoomFor(s, max + 1);
    for (size_t i = 0; i < max; i++) {
        int c = fgetc(fp);
        if (c == EOF ||
            c == '\n')
        {
            s[i] = '\0';
            sdstrim(s, "\r \t");
            *nread = c == EOF
                ? -1
                : (int)sdslen(s);
            return s;
        }
        s[i] = (char)c;
        sdsinclen(s, 1);
    }
    MYGPIOD_LOG_ERROR("Line is too long, max length is %lu", (unsigned long)max);
    s[max] = '\0';
    sdstrim(s, "\r \t");

    *nread = (int)sdslen(s);
    return s;
}

/**
 * Reads a whole file in a sds string
 * @param s an already allocated sds string to append the file content
 * @param file_path filename to read
 * @param nread Number of bytes read,
 *              -1 error reading file
 * @return pointer to s
 */
sds sds_getfile(sds s, const char *file_path, int *nread) {
    errno = 0;
    FILE *fp = fopen(file_path, OPEN_FLAGS_READ);
    if (fp == NULL) {
        *nread = -1;
        MYGPIOD_LOG_ERROR("Unable to open file \"%s\": %s", file_path, strerror(errno));
        return s;
    }
    const size_t buffer_size = 10240;
    s = sdsMakeRoomFor(s, buffer_size);
    size_t read;
    while ((read = fread(s + sdslen(s), sizeof(char), buffer_size, fp)) > 0) {
        sdsIncrLen(s, (ssize_t)read);
        s = sdsMakeRoomFor(s, buffer_size);
    }
    if (ferror(fp)) {
        MYGPIOD_LOG_ERROR("Error reading file \"%s\"", file_path);
    }
    (void) fclose(fp);
    *nread = (int)sdslen(s);
    return s;
}

/**
 * Splits the string into two parts by first occurrence of sep.
 * Trims whitespaces from start and end of the tokens
 * @param s sds string to split
 * @param sep separator char
 * @param count pointer to populate the 
 * @return allocated array of tokens or NULL on error
 */
sds *sds_splitfirst(sds s, char sep, int *count) {
    size_t len = sdslen(s);
    sds *tokens = malloc_assert(sizeof(sds)*2);
    size_t start = 0;
    int elements = 0;
    for (size_t i = 0; i < len; i++) {
        if (*(s + i) == sep) {
            tokens[elements] = sdsnewlen(s, i);
            sdstrim(tokens[elements], " \t");
            elements++;
            start = i + 1;
            break;
        }
    }

    tokens[elements] = sdsnewlen(s + start, len - start);
    sdstrim(tokens[elements], " \t");
    elements++;
    *count = elements;
    return tokens;
}

/**
 * Parses a string to a GPIO value.
 * Sets errno to EINVAL on parser error.
 * @param str string to parse
 * @return GPIO value or GPIOD_LINE_VALUE_ERROR on error
 */
enum gpiod_line_value parse_gpio_value(const char *str) {
    if (strcasecmp(str, "active") == 0) {
        return GPIOD_LINE_VALUE_ACTIVE;
    }
    if (strcasecmp(str, "inactive") == 0) {
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
 * Lookups the string for a GPIO value.
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
 * Parses a string to a GPIO drive setting.
 * Sets errno to EINVAL on parser error.
 * @param str string to parse
 * @return GPIO value or GPIO_VALUE_LOW on error
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
    errno = EINVAL;
    MYGPIOD_LOG_WARN("Could not parse gpio value, setting push-pull");
    return GPIOD_LINE_DRIVE_PUSH_PULL;
}

/**
 * Lookups the string for a GPIO drive
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
 * Parses the bias flags
 * Sets errno to EINVAL on parser error.
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
    errno = EINVAL;
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
 * Sets errno to EINVAL on parser error.
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
    errno = EINVAL;
    MYGPIOD_LOG_WARN("Could not parse event request, setting monotonic");
    return GPIOD_LINE_CLOCK_MONOTONIC;
}

/**
 * Lookups the event clock name
 * @param clock the event clock
 * @return name
 */
const char *lookup_event_clock(enum gpiod_line_clock clock) {
    switch(clock) {
        case GPIOD_LINE_CLOCK_REALTIME:
            return "realtime";
        case GPIOD_LINE_CLOCK_HTE:
            return "hte";
        case GPIOD_LINE_CLOCK_MONOTONIC:
            return "monotonic";
    }
    MYGPIOD_LOG_WARN("Could not parse event clock, setting monotonic");
    return "monotonic";
}

/**
 * Parses the request event setting.
 * Sets errno to EINVAL on parser error.
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
    errno = EINVAL;
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
            return "rising";
        case GPIOD_LINE_EDGE_FALLING:
            return "falling";
        case GPIOD_LINE_EDGE_BOTH:
            return "both";
    }
    MYGPIOD_LOG_WARN("Could not lookup event request, setting none");
    return "none";
}

/**
 * Parses the event type.
 * Sets errno to EINVAL on parser error.
 * @param str string to parse
 * @return gpiod_edge_event_type enum
 */
enum gpiod_edge_event_type parse_event_type(const char *str) {
    if (strcasecmp(str, "falling") == 0) {
        return GPIOD_EDGE_EVENT_FALLING_EDGE;
    }
    if (strcasecmp(str, "rising") == 0) {
        return GPIOD_EDGE_EVENT_RISING_EDGE;
    }
    errno = EINVAL;
    MYGPIOD_LOG_WARN("Could not parse event type, setting falling");
    return GPIOD_EDGE_EVENT_FALLING_EDGE;
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

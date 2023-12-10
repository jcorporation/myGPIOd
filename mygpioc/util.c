/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "mygpioc/util.h"

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Parses the string to an integer value and checks it against min and max.
 * @param str string to parse
 * @param result pointer for the result
 * @param rest pointer to first none numeric char
 * @param min minimum value (including)
 * @param max maximum value (including)
 * @return bool true on success, else false
 */
bool parse_int(const char *str, int *result, int min, int max) {
    errno = 0;
    char *endptr;
    intmax_t v = strtoimax(str, &endptr, 10);
    if (endptr != NULL &&
        endptr != str &&
        errno == 0 &&
        v >= min &&
        v <= max)
    {
        *result = (int)v;
        return true;
    }
    return false;
}

/**
 * Parses the string to an unsigned integer value and checks it against min and max.
 * @param str string to parse
 * @param result pointer for the result
 * @param rest pointer to first none numeric char
 * @param min minimum value (including)
 * @param max maximum value (including)
 * @return bool true on success, else false
 */
bool parse_uint(const char *str, unsigned *result, unsigned min, unsigned max) {
    errno = 0;
    char *endptr;
    uintmax_t v = strtoumax(str, &endptr, 10);
    if (endptr != NULL &&
        endptr != str &&
        errno == 0 &&
        v >= min &&
        v <= max)
    {
        *result = (unsigned)v;
        return true;
    }
    return false;
}

/**
 * Prints a message if verbose is true
 * @param fmt 
 * @param ... 
 */
void verbose_printf(const char *fmt, ...) {
    if (verbose == false) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    vprintf(fmt, args);
    va_end(args);
    #pragma GCC diagnostic pop
    printf("\n");
}

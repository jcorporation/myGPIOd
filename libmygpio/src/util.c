/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/src/util.h"

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Parses the start of a string to an unsigned integer value and checks it against min and max.
 * @param str string to parse
 * @param result pointer for the result
 * @param rest pointer to first none numeric char
 * @param min minimum value (including)
 * @param max maximum value (including)
 * @return bool true on success, else false
 */
bool parse_uint(const char *str, unsigned *result, char **rest, unsigned min, unsigned max) {
    errno = 0;
    uintmax_t v = strtoumax(str, rest, 10);
    if (errno == 0 && v >= min && v <= max) {
        *result = (int)v;
        return true;
    }
    return false;
}

/**
 * Parses the start of a string to an uint64_t value.
 * @param str string to parse
 * @param result pointer for the result
 * @return bool true on success, else false
 */
bool parse_uint64(const char *str, uint64_t *result) {
    errno = 0;
    unsigned long long v = strtoull(str, NULL, 10);
    if (errno == 0) {
        *result = v;
        return true;
    }
    return false;
}

#ifdef MYGPIOD_DEBUG
/**
 * Debug logging function. Do not call it directly, use the LIBMYGPIO_LOG macro.
 * @param file file of the log statement
 * @param line line number of the log statement
 * @param fmt format string
 * @param ... variadic arguments for format string
 */
void log_log(const char *file, int line, const char *fmt, ...) {
    printf("%s:%d: ", file, line);
    va_list args;
    va_start(args, fmt);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    vprintf(fmt, args);
    va_end(args);
    #pragma GCC diagnostic pop
    printf("\n");

}
#endif

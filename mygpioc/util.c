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
 * Parses the start of a string to an integer value and checks it against min and max.
 * @param str string to parse
 * @param result pointer for the result
 * @param rest pointer to first none numeric char
 * @param min minimum value (including)
 * @param max maximum value (including)
 * @return bool true on success, else false
 */
bool parse_int(const char *str, int *result, char **rest, int min, int max) {
    errno = 0;
    intmax_t v = strtoimax(str, rest, 10);
    if (errno == 0 && v >= min && v <= max) {
        *result = (int)v;
        return true;
    }
    return false;
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_SRC_UTIL_H
#define LIBMYGPIO_SRC_UTIL_H

#include <stdbool.h>
#include <stdint.h>

bool libmygpio_parse_uint(const char *str, unsigned *result, char **rest, unsigned min, unsigned max);
bool libmygpio_parse_uint64(const char *str, uint64_t *result);

#ifdef MYGPIOD_DEBUG
    #define LIBMYGPIO_LOG(...) libmygpio_log_log(__FILE__, __LINE__, __VA_ARGS__)
    void libmygpio_log_log(const char *file, int line, const char *fmt, ...);
#endif

#endif

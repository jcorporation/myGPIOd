/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_UTIL_H
#define LIBMYGPIO_UTIL_H

#include <stdbool.h>

bool parse_int(const char *str, int *result, char **rest, int min, int max);

#ifdef MYGPIOD_DEBUG
    #define LIBMYGPIO_LOG(...) log_log(__FILE__, __LINE__, __VA_ARGS__)
    void log_log(const char *file, int line, const char *fmt, ...);
#endif

#endif

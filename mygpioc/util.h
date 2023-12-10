/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_UTIL_H
#define MYGPIOC_UTIL_H

#include <stdbool.h>
#include <stdint.h>

extern bool verbose;

bool parse_int(const char *str, int *result, int min, int max);
bool parse_uint(const char *str, unsigned *result, unsigned min, unsigned max);
void verbose_printf(const char *fmt, ...);

#endif

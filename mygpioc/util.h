/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_UTIL_H
#define MYGPIOC_UTIL_H

#include <stdbool.h>
#include <stdint.h>

bool parse_int(const char *str, int *result, char **rest, int min, int max);

#endif

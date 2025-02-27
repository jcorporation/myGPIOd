/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_UTIL_H
#define MYGPIOC_UTIL_H

#include <stdbool.h>
#include <stdint.h>

extern bool verbose;

void verbose_printf(const char *fmt, ...);

#endif

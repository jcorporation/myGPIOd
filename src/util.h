/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_UTIL_H
#define MYGPIOD_UTIL_H

#include <stdbool.h>
#include <stddef.h>

int make_signalfd(void);
bool value_in_array(unsigned value, unsigned *array, size_t len);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_RASPBERRY_H
#define MYGPIOD_RASPBERRY_H

#include "dist/sds/sds.h"
#include <stdbool.h>

sds vcgencmd(const char *command, sds buffer, bool *rc);

#endif

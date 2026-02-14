/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_LUA_LUAVM_H
#define MYGPIOD_LUA_LUAVM_H

#include "mygpiod/config/config.h"

#include <stdbool.h>

bool luavm_init(struct t_config *config);

#endif

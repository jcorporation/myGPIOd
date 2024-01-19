/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_ACTIONS_LUA_H
#define MYGPIOD_ACTIONS_LUA_H

#include "mygpiod/lib/config.h"

#include <stdbool.h>

bool action_lua(struct t_config *config, const char *cmd);
bool init_luavm(struct t_config *config);

#endif

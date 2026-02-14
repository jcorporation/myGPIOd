/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_ACTIONS_LUA_H
#define MYGPIOD_ACTIONS_LUA_H

#include "mygpiod/config/config.h"
#include "mygpiod/lib/action.h"

#include <stdbool.h>

bool action_lua(struct t_config *config, struct t_action *action);

#endif

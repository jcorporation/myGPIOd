/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief GPIO actions
 */

#ifndef MYGPIOD_ACTIONS_GPIOSET_H
#define MYGPIOD_ACTIONS_GPIOSET_H

#include "mygpiod/config/config.h"
#include "mygpiod/lib/action.h"

#include <stdbool.h>

bool action_gpioset(struct t_config *config, struct t_action *action);
bool action_gpiotoggle(struct t_config *config, struct t_action *action);
bool action_gpioblink(struct t_config *config, struct t_action *action);

#endif

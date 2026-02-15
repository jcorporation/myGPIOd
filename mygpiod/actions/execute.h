/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Actions execute handler
 */

#ifndef MYGPIOD_ACTIONS_EXECUTE_H
#define MYGPIOD_ACTIONS_EXECUTE_H

#include "mygpiod/lib/action.h"
#include "mygpiod/config/config.h"

void actions_execute(struct t_config *config, struct t_list *actions);
void action_execute(struct t_config *config, struct t_action *action);

#endif

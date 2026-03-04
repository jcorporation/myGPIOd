/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Hook action handling
 */

#ifndef MYGPIOD_HOOK_ACTION_H
#define MYGPIOD_HOOK_ACTION_H

#include "mygpiod/config/config.h"

bool hook_action_handler(struct t_config *config, const char *hook_name);

#endif

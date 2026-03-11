/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Execute system actions
 */

#ifndef MYGPIOD_ACTIONS_SYSTEM_H
#define MYGPIOD_ACTIONS_SYSTEM_H

#include <stdbool.h>

bool action_system_async(const char *cmd);

#endif

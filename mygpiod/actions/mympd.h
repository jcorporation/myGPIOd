/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief myMPD actions
 */

#ifndef MYGPIOD_ACTIONS_MYMPD_H
#define MYGPIOD_ACTIONS_MYMPD_H

#include "mygpiod/actions/actions.h"

#include <stdbool.h>

bool action_mympd(struct t_action *action);
bool action_mympd2(const char *uri, const char *partition, const char *script);

#endif

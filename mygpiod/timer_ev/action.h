/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Timer event action handling
 */

#ifndef MYGPIOD_TIMER_EV_ACTION_H
#define MYGPIOD_TIMER_EV_ACTION_H

#include "mygpiod/timer_ev/timer_ev.h"
#include "mygpiod/config/config.h"

void timer_ev_action_handle(struct t_config *config, struct t_timer_definition *timer_definition);

#endif

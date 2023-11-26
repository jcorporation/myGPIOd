/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_ACTION_H
#define MYGPIOD_ACTION_H

#include "config.h"
#include "event.h"

#include <time.h>

void action_handle(unsigned gpio, const struct timespec *ts, int event_type, struct t_config *config);
void action_delay_abort(struct t_config *config);
void action_execute_delayed(struct t_mon_ctx *ctx);

#endif

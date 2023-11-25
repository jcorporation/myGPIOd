/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_ACTION_H
#define MYGPIOD_ACTION_H

#include "config.h"

#include <time.h>

void action_execute(unsigned int offset, const struct timespec *ts, int event_type, struct t_config *config);

#endif

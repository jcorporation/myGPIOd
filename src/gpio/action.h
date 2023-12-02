/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_GPIO_ACTION_H
#define MYGPIOD_GPIO_ACTION_H

#include "src/lib/config.h"

#include <time.h>

void action_handle(unsigned gpio, const struct timespec *ts, int event_type, struct t_gpio_node_in *node);
void action_delay_abort(struct t_gpio_node_in *node);
void action_execute_delayed(unsigned gpio, struct t_gpio_node_in *node, struct t_config *config);

#endif

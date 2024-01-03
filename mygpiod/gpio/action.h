/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_GPIO_ACTION_H
#define MYGPIOD_GPIO_ACTION_H

#include "mygpiod/lib/config.h"

#include <stdint.h>
#include <time.h>

void action_handle(struct t_config *config, unsigned gpio, uint64_t timestamp,
        enum gpiod_edge_event_type event_type, struct t_gpio_in_data *data);
void action_delay_abort(struct t_gpio_in_data *data);
void action_execute_delayed(unsigned gpio, struct t_gpio_in_data *data, struct t_config *config);

#endif

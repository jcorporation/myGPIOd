/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_INPUT_H
#define MYGPIOD_GPIO_INPUT_H

#include "mygpiod/lib/config.h"
#include "mygpiod/event_loop/event_loop.h"

#include <gpiod.h>

bool gpio_request_inputs(struct t_config *config, struct t_poll_fds *poll_fds);
bool gpio_request_input(struct gpiod_chip *chip, unsigned gpio, struct t_gpio_in_data *data);

#endif

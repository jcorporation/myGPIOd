/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_INPUT_H
#define MYGPIOD_GPIO_INPUT_H

#include "mygpiod/lib/config.h"

#include <gpiod.h>

enum gpiod_line_value gpio_get_value(struct t_config *config, unsigned gpio);

#endif

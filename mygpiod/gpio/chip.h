/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_CHIP_H
#define MYGPIOD_GPIO_CHIP_H

#include "mygpiod/lib/config.h"

bool gpio_open_chip(struct t_config *config);

#endif

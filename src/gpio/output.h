/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_OUTPUT_H
#define MYGPIOD_GPIO_OUTPUT_H

#include "src/lib/config.h"

#include <gpiod.h>

bool gpio_set_outputs(struct t_config *config);
bool gpio_set_output(struct gpiod_chip *chip, unsigned gpio, struct t_gpio_out_data *data);

bool gpio_set_value(struct t_config *config, unsigned gpio, enum gpiod_line_value value);

#endif

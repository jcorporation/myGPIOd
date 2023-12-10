/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_SRC_GPIO_H
#define LIBMYGPIO_SRC_GPIO_H

#include "libmygpio/include/libmygpio/gpio.h"

/**
 * Struct holding the configuration of a GPIO.
 */
struct t_mygpio_gpio_conf {
    unsigned gpio;               //!< GPIO number
    enum mygpio_gpio_mode mode;  //!< GPIO mode
};

#endif

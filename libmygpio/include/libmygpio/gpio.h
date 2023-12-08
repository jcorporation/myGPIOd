/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

// Do not include this file directly, use libmygpio.h

#ifndef LIBMYGPIO_GPIO_H
#define LIBMYGPIO_GPIO_H

#include <stdbool.h>

struct t_mygpio_connection;

enum mygpio_gpio_mode {
    MYGPIO_GPIO_MODE_UNKNOWN = -1,
    MYGPIO_GPIO_MODE_OUT,
    MYGPIO_GPIO_MODE_IN
};

struct t_mygpio_gpio_conf {
    unsigned gpio;
    enum mygpio_gpio_mode mode;
};

bool mygpio_gpiolist(struct t_mygpio_connection *connection);
struct t_mygpio_gpio_conf *mygpio_recv_gpio(struct t_mygpio_connection *connection);
void mygpio_free_gpio(struct t_mygpio_gpio_conf *gpio_conf);

#endif

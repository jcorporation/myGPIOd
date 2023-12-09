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
    MYGPIO_GPIO_MODE_IN,
    MYGPIO_GPIO_MODE_OUT
};

enum mygpio_gpio_value {
    MYGPIO_GPIO_VALUE_UNKNOWN = -1,
    MYGPIO_GPIO_VALUE_HIGH,
    MYGPIO_GPIO_VALUE_LOW
};

struct t_mygpio_gpio_conf {
    unsigned gpio;
    enum mygpio_gpio_mode mode;
};

bool mygpio_gpiolist(struct t_mygpio_connection *connection);
struct t_mygpio_gpio_conf *mygpio_recv_gpio_conf(struct t_mygpio_connection *connection);
void mygpio_free_gpio_conf(struct t_mygpio_gpio_conf *gpio_conf);
enum mygpio_gpio_value mygpio_gpioget(struct t_mygpio_connection *connection, unsigned gpio);
bool mygpio_gpioset(struct t_mygpio_connection *connection, unsigned gpio, enum mygpio_gpio_value value);

#endif

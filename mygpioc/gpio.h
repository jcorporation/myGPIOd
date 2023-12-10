/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_GPIO_H
#define MYGPIOC_GPIO_H

#include "libmygpio/include/libmygpio/libmygpio.h"

int handle_gpiolist(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_gpioget(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_gpioset(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);

#endif

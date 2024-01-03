/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_GPIO_H
#define MYGPIOC_GPIO_H

struct t_mygpio_connection;

int handle_gpioinfo(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_gpiolist(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_gpioget(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_gpioset(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_gpiotoggle(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_gpioblink(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_RASPBERRY_VCIO_H
#define MYGPIOC_RASPBERRY_VCIO_H

struct t_mygpio_connection;

int handle_vciotemp(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_vciovolts(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_vcioclock(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);
int handle_vciothrottled(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);

#endif

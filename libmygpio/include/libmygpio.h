/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIOD_H
#define LIBMYGPIOD_H

struct t_mygpio_connection;

struct t_mygpio_connection *mygpio_connection_new(const char *socket_path);
void mygpio_connection_free(struct t_mygpio_connection *connection);

#endif

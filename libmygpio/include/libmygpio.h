/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_H
#define LIBMYGPIO_H

struct t_mygpio_connection;

struct t_mygpio_pair {
    const char *name;
    const char *value;
};

struct t_mygpio_connection *mygpio_connection_new(const char *socket_path);
void mygpio_connection_free(struct t_mygpio_connection *connection);
const char *mygpio_connection_version(struct t_mygpio_connection *connection);

#endif

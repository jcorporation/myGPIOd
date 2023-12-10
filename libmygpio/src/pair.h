/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIO_SRC_PAIR_H
#define MYGPIO_SRC_PAIR_H

struct t_mygpio_connection;

struct t_mygpio_pair {
    const char *name;
    const char *value;
};

struct t_mygpio_pair *mygpio_recv_pair(struct t_mygpio_connection *connection);
void mygpio_free_pair(struct t_mygpio_pair *pair);

#endif

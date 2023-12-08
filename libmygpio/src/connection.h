/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_SRC_CONNECTION_H
#define LIBMYGPIO_SRC_CONNECTION_H

#include "libmygpio/include/libmygpio/connection.h"
#include "libmygpio/src/buffer.h"

struct t_mygpio_connection {
    int fd;
    char *socket_path;
    struct t_buf buf_in;
    struct t_buf buf_out;
    unsigned version[3];
    int timeout;
    enum mygpio_conn_state state;
    char *error;
};

void connection_set_state(struct t_mygpio_connection *connection,
        enum mygpio_conn_state state, const char *message);

#endif

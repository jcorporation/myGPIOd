/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_CONNECTION_H
#define LIBMYGPIO_CONNECTION_H

#include "libmygpio/src/buffer.h"

enum mygpio_conn_state {
    MYGPIO_STATE_OK,
    MYGPIO_STATE_ERROR,
    MYGPIO_STATE_FATAL
};

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

struct t_mygpio_connection *mygpio_connection_new(const char *socket_path);
void mygpio_connection_free(struct t_mygpio_connection *connection);
int mygpio_connection_get_fd(struct t_mygpio_connection *connection);
void connection_set_state(struct t_mygpio_connection *connection,
        enum mygpio_conn_state state, const char *message);
const unsigned *mygpio_connection_get_version(struct t_mygpio_connection *connection);
enum mygpio_conn_state mygpio_connection_get_state(struct t_mygpio_connection *connection);
const char *mygpio_connection_get_error(struct t_mygpio_connection *connection);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_H
#define LIBMYGPIO_H

#include <stdbool.h>

enum mygpio_conn_state {
    MYGPIO_STATE_OK,
    MYGPIO_STATE_ERROR,
    MYGPIO_STATE_FATAL
};

struct t_mygpio_connection;

struct t_mygpio_pair {
    const char *name;
    const char *value;
};

struct t_mygpio_connection *mygpio_connection_new(const char *socket_path);
void mygpio_connection_free(struct t_mygpio_connection *connection);
const unsigned *mygpio_connection_get_version(struct t_mygpio_connection *connection);
int mygpio_connection_get_fd(struct t_mygpio_connection *connection);
enum mygpio_conn_state mygpio_connection_get_state(struct t_mygpio_connection *connection);
const char *mygpio_connection_get_error(struct t_mygpio_connection *connection);
bool mygpio_connection_clear_error(struct t_mygpio_connection *connection);

struct t_mygpio_pair *mygpio_recv_pair(struct t_mygpio_connection *connection);
void mygpio_free_pair(struct t_mygpio_pair *pair);

bool mygpio_send_idle(struct t_mygpio_connection *connection);
bool mygpio_recv_idle(struct t_mygpio_connection *connection, int timeout);
bool mygpio_send_noidle(struct t_mygpio_connection *connection);

#endif

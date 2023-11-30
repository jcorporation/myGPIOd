/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_SOCKET_H
#define MYGPIOD_SERVER_SOCKET_H

#include "config.h"

#include <poll.h>
#include <sys/types.h>

/**
 * Socket states
 */
enum client_socket_state {
    CLIENT_SOCKET_STATE_READING,
    CLIENT_SOCKET_STATE_WRITING
};

/**
 * Client data
 */
struct t_client_data {
    int fd;                          //!< client file descriptor
    enum client_socket_state state;  //!< internal socket state
    char buf_in[4096];               //!< incoming buffer
    char buf_out[4096];              //!< outgoing buffer
    ssize_t bytes_in;                //!< read bytes
    ssize_t bytes_out;               //!< bytes already written
    short events;                    //!< events to poll
};

int socket_create(struct t_config *config);
bool server_accept_client_connection(struct t_config *config, int *server_fd);
bool server_handle_client_connection(struct t_config *config, struct pollfd *client_fd);
void server_free_free_client_connection(struct t_list_node *node);

#endif

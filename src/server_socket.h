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
    CLIENT_SOCKET_STATE_IDLE,
    CLIENT_SOCKET_STATE_WRITING
};

#define SERVER_INPUT_BUFFER_SIZE 1024
#define SERVER_OUTPUT_BUFFER_SIZE 4096

/**
 * Client data
 */
struct t_client_data {
    int fd;                                   //!< client file descriptor
    enum client_socket_state state;           //!< internal socket state
    char buf_in[SERVER_INPUT_BUFFER_SIZE];    //!< incoming buffer
    char buf_out[SERVER_OUTPUT_BUFFER_SIZE];  //!< outgoing buffer
    size_t buf_out_len;                       //!< current length of the output buffer
    ssize_t bytes_in;                         //!< read bytes
    ssize_t bytes_out;                        //!< bytes already written
    short events;                             //!< events to poll
    struct t_list waiting_events;             //!< waiting events
};

int server_socket_create(struct t_config *config);
bool server_client_connection_accept(struct t_config *config, int *server_fd);
bool server_client_connection_handle(struct t_config *config, struct pollfd *client_fd);
void server_client_disconnect(struct t_list *clients, struct t_list_node *node);
void server_client_connection_clear(struct t_list_node *node);

void server_send_response(struct t_list_node *node, const char *message);
#endif

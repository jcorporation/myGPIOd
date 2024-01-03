/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_SOCKET_H
#define MYGPIOD_SERVER_SOCKET_H

#include "dist/sds/sds.h"
#include "mygpiod/lib/config.h"

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

#define BUFFER_SIZE_INPUT_MAX 4096
#define BUFFER_SIZE 1024

/**
 * Client data
 */
struct t_client_data {
    int fd;                          //!< client file descriptor
    enum client_socket_state state;  //!< internal socket state
    sds buf_in;                      //!< incoming buffer
    sds buf_out;                     //!< outgoing buffer
    ssize_t bytes_out;               //!< bytes written to socket
    short events;                    //!< events to poll
    struct t_list waiting_events;    //!< waiting events
    int timeout_fd;                  //!< timer fd for socket timeout
};

int server_socket_create(struct t_config *config);
bool server_client_connection_accept(struct t_config *config, int *server_fd);
bool server_client_connection_handle(struct t_config *config, struct pollfd *client_fd);
bool server_client_disconnect(struct t_list *clients, struct t_list_node *node);
struct t_client_data *server_client_connection_new(int client_fd);
void server_client_connection_clear(struct t_list_node *node);
int server_client_connection_set_timeout(int timeout_fd, int timeout_s);
void server_client_connection_remove_timeout(struct t_client_data *data);
bool server_client_timeout(struct t_list *clients, int *timeout_fd);

#endif

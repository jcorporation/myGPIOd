/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server/socket.h"

#include "dist/sds/sds.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/lib/events.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/timer.h"
#include "mygpiod/lib/util.h"
#include "mygpiod/server/protocol.h"
#include "mygpiod/server/response.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

// private definitions

static struct t_list_node *get_node_by_clientfd(struct t_list *clients, int *fd);
static struct t_list_node *get_node_by_timeoutfd(struct t_list *clients, int *fd);

// public functions

/**
 * Creates the server socket
 * @param config pointer to config
 * @return the creates socket fd or -1 on error
 */
int server_socket_create(struct t_config *config) {
    MYGPIOD_LOG_INFO("Creating server socket \"%s\"", config->socket_path);
    struct sockaddr_un address = { 0 };
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, config->socket_path, 108);
    unlink(config->socket_path);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        MYGPIOD_LOG_ERROR("Can not create socket \"%s\"", config->socket_path);
        return -1;
    }

    int flags = fcntl(fd, F_GETFD, 0);
    if (fcntl(fd, F_SETFD, flags | O_NONBLOCK | O_CLOEXEC)) {
        MYGPIOD_LOG_ERROR("Can not set socket options");
        close(fd);
        return -1;
    }

    errno = 0;
    if (bind(fd, (struct sockaddr *)(&address), sizeof(address)) == -1) {
        MYGPIOD_LOG_ERROR("Can not bind to socket \"%s\": %s", config->socket_path, strerror(errno));
        close(fd);
        return -1;
    }

    if (listen(fd, 10) < 0) {
        MYGPIOD_LOG_ERROR("Can not listen on socket \"%s\"", config->socket_path);
        close(fd);
        return -1;
    }

    return fd;
}

/**
 * Accepts a new client connection
 * @param config pointer to config
 * @param server_fd server file descriptor
 * @return true on success, else false
 */
bool server_client_connection_accept(struct t_config *config, int *server_fd) {
    int client_fd = accept(*server_fd, NULL, NULL);
    if (client_fd < 0) {
        MYGPIOD_LOG_ERROR("Error creating client socket");
        return false;
    }
    if (config->clients.length == CLIENT_CONNECTIONS_MAX) {
        close(client_fd);
        MYGPIOD_LOG_ERROR("Client connection limit reached");
        return false;
    }
    int flags = fcntl(client_fd, F_GETFD, 0);
    if (fcntl(client_fd, F_SETFD, flags | O_NONBLOCK | O_CLOEXEC)) {
        MYGPIOD_LOG_ERROR("Can not set socket options");
        close(client_fd);
        return false;
    }

    struct t_client_data *data = server_client_connection_new(client_fd);
    config->client_id++;
    list_push(&config->clients, config->client_id, data);
    MYGPIOD_LOG_INFO("Client#%u: Accepted new connection", config->client_id);
    server_response_send(data, DEFAULT_MSG_OK "\nversion:" MYGPIO_VERSION "\n" DEFAULT_MSG_END);
    data->timeout_fd = server_client_connection_set_timeout(data->timeout_fd, config->socket_timeout_s);
    timer_log_next_expire(data->timeout_fd);
    return true;
}

/**
 * Handles client connections
 * @param config pointer to config
 * @param client_fd client poll fd
 * @return true on success, else false
 */
bool server_client_connection_handle(struct t_config *config, struct pollfd *client_fd) {
    struct t_list_node *node = get_node_by_clientfd(&config->clients, &client_fd->fd);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Could not find fd in connection table");
        return false;
    }
    struct t_client_data *data = (struct t_client_data *)node->data;

    if (client_fd->revents & POLLHUP) {
        MYGPIOD_LOG_DEBUG("Client#%u: POLLHUP received", node->id);
        server_client_disconnect(&config->clients, node);
        return true;
    }
    if ((client_fd->revents & POLLERR) ||
        (client_fd->revents & POLLNVAL))
    {
        MYGPIOD_LOG_WARN("Client#%u: Socket error", node->id);
        server_client_disconnect(&config->clients, node);
        return false;
    }

    switch(data->state) {
        case CLIENT_SOCKET_STATE_READING:
        case CLIENT_SOCKET_STATE_IDLE: {
            size_t oldlen = sdslen(data->buf_in);
            data->buf_in = sdsMakeRoomFor(data->buf_in, BUFFER_SIZE);
            ssize_t nread = read(data->fd, data->buf_in + oldlen, BUFFER_SIZE);
            if (nread <= 0) {
                MYGPIOD_LOG_DEBUG("Client#%u: Could not read from socket", node->id);
                server_client_disconnect(&config->clients, node);
                return false;
            }
            sdsIncrLen(data->buf_in, nread);
            char *buf_end = memchr(data->buf_in, '\n', sdslen(data->buf_in));
            if (buf_end != NULL) {
                sdstrim(data->buf_in, " \t \n");
                MYGPIOD_LOG_DEBUG("Client#%u: Read line \"%s\"", node->id, data->buf_in);
                data->timeout_fd = server_client_connection_set_timeout(data->timeout_fd, config->socket_timeout_s);
                timer_log_next_expire(data->timeout_fd);
                server_protocol_handler(config, node);
                return true;
            }
            if (sdslen(data->buf_in) >= BUFFER_SIZE_INPUT_MAX) {
                MYGPIOD_LOG_ERROR("Client#%u: Request line too long", node->id);
                server_client_disconnect(&config->clients, node);
                return false;
            }
            return true;
        }
        case CLIENT_SOCKET_STATE_WRITING: {
            size_t max_bytes = sdslen(data->buf_out) - (size_t)data->bytes_out;
            ssize_t result = write(data->fd, data->buf_out + data->bytes_out, max_bytes);
            if (result < 0) {
                MYGPIOD_LOG_ERROR("Client#%u: Could not write to socket", node->id);
                server_client_disconnect(&config->clients, node);
                return false;
            }
            data->bytes_out += result;
            if ((size_t)result == max_bytes) {
                data->state = CLIENT_SOCKET_STATE_READING;
                data->events = POLLIN;
                sdsclear(data->buf_out);
            }
            return true;
        }
    }
    MYGPIOD_LOG_WARN("Unknown client socket state: \"%d\"", data->state);
    return false;
}

/**
 * Creates the client connection data
 * @param client_fd client connection fd
 * @return allocated client connection data
 */
struct t_client_data *server_client_connection_new(int client_fd) {
    struct t_client_data *data = malloc_assert(sizeof(struct t_client_data));
    data->fd = client_fd;
    data->timeout_fd = -1;
    data->state = CLIENT_SOCKET_STATE_WRITING;
    data->buf_in = sdsempty();
    data->buf_out = sdsempty();
    list_init(&data->waiting_events);
    update_pollfds = true;
    return data;
}

/**
 * Closes the client fd and frees the connection
 * @param node pointer to client
 */
void server_client_connection_clear(struct t_list_node *node) {
    struct t_client_data *data = (struct t_client_data *)node->data;
    close_fd(&data->fd);
    close_fd(&data->timeout_fd);
    FREE_SDS(data->buf_in);
    FREE_SDS(data->buf_out);
    list_clear(&data->waiting_events, event_data_clear);
    update_pollfds = true;
}

/**
 * Removes the client connection from the connection list
 * and closes and frees it.
 * @param clients pointer to client list
 * @param node node holding the client data to remove
 * @return true on success, else false
 */
bool server_client_disconnect(struct t_list *clients, struct t_list_node *node) {
    MYGPIOD_LOG_INFO("Client#%u: Connection closed", node->id);
    if (list_remove_node(clients, node) == false) {
        MYGPIOD_LOG_ERROR("Could not find client connection");
        return false;
    }
    server_client_connection_clear(node);
    FREE_PTR(node->data);
    FREE_PTR(node);
    return true;
}

/**
 * Adds/replaces a socket timeout handler
 * @param timeout_fd socket
 * @param timeout_s timeout in seconds
 */
int server_client_connection_set_timeout(int timeout_fd, int timeout_s) {
    if (timeout_fd > 0) {
        timer_set(timeout_fd, timeout_s * 1000, 0);
    }
    return timer_new(timeout_s * 1000, 0);
}

/**
 * Removes a socket timeout handler
 * @param data client data
 */
void server_client_connection_remove_timeout(struct t_client_data *data) {
    close_fd(&data->timeout_fd);
}

/**
 * Disconnects a client after timeout expired
 * @param clients pointer to client list
 * @param fd timeout timer fd
 */
bool server_client_timeout(struct t_list *clients, int *timeout_fd) {
    timer_log_next_expire(*timeout_fd);
    struct t_list_node *node = get_node_by_timeoutfd(clients, timeout_fd);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("No timeout fd found");
        return false;
    }
    MYGPIOD_LOG_INFO("Client#%u: Timeout", node->id);
    server_client_disconnect(clients, node);
    return true;
}

// private functions

/**
 * Gets the client node by timer_fd
 * @param clients list of clients
 * @param fd client fd
 * @return the list node or NULL on error
 */
static struct t_list_node *get_node_by_clientfd(struct t_list *clients, int *fd) {
    struct t_list_node *current = clients->head;
    while (current != NULL) {
        struct t_client_data *data = (struct t_client_data *)current->data;
        if (data->fd == *fd) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Gets the client node by timeout_fd
 * @param clients list of clients
 * @param fd timeout_fd
 * @return the list node or NULL on error
 */
static struct t_list_node *get_node_by_timeoutfd(struct t_list *clients, int *fd) {
    struct t_list_node *current = clients->head;
    while (current != NULL) {
        struct t_client_data *data = (struct t_client_data *)current->data;
        if (data->timeout_fd == *fd) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

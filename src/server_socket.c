/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "server_socket.h"

#include "log.h"
#include "src/util.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// private definitions

#define WELCOME_MESSAGE "OK myGPIOd " MYGPIOD_VERSION "\n"
#define CLIENT_ID_MAX 9999

static struct t_list_node *get_node_by_clientfd(struct t_list *clients, int *fd);

// public functions

/**
 * Creates the server socket
 * @param config pointer to config
 * @return the creates socket fd or -1 on error
 */
int socket_create(struct t_config *config) {
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
    if (fcntl(fd, F_SETFD, flags | O_NONBLOCK)) {
        MYGPIOD_LOG_ERROR("Can not set socket to nonblock");
        return -1;
    }

    errno = 0;
    if (bind(fd, (struct sockaddr *)(&address), sizeof(address)) == -1) {
        MYGPIOD_LOG_ERROR("Can not bind to socket \"%s\": %s", config->socket_path, strerror(errno));
        return -1;
    }

    if (listen(fd, 10) < 0) {
        MYGPIOD_LOG_ERROR("Can not listen on socket \"%s\"", config->socket_path);
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
bool server_accept_client_connection(struct t_config *config, int *server_fd) {
    //TODO: connection limit
    int client_fd = accept(*server_fd, NULL, NULL);
    if (client_fd < 0) {
        MYGPIOD_LOG_ERROR("Error creating client socket");
        return false;
    }
    int flags = fcntl(client_fd, F_GETFD, 0);
    if (fcntl(client_fd, F_SETFD, flags | O_NONBLOCK)) {
        MYGPIOD_LOG_ERROR("Can not set socket to nonblock");
        return false;
    }
    struct t_client_data *data = malloc(sizeof(struct t_client_data));
    if (data == NULL) {
        return false;
    }

    data->fd = client_fd;
    data->state = CLIENT_SOCKET_STATE_WRITING;
    memset(data->buf_in, 0, sizeof(data->buf_in));
    memset(data->buf_out, 0, sizeof(data->buf_out));
    data->bytes_in = 0;
    data->bytes_out = 0;
    data->events = POLLOUT;
    strcpy(data->buf_out, WELCOME_MESSAGE);
    config->client_id++;
    list_push(&config->clients, config->client_id, data);
    MYGPIOD_LOG_DEBUG("Accepted new client connection: %u", config->clients.length);
    return true;
}

/**
 * Closes the client fd and frees the connection
 * @param node pointer to client
 */
void server_free_free_client_connection(struct t_list_node *node) {
    struct t_client_data *data = (struct t_client_data *)node->data;
    if (data->fd > 0) {
        close(data->fd);
    }
    free(data);
    free(node);
}

/**
 * Handles client connections
 * @param config pointer to config
 * @param client_fd client poll fd
 * @return true on success, else false
 */
bool server_handle_client_connection(struct t_config *config, struct pollfd *client_fd) {
    struct t_list_node *node = get_node_by_clientfd(&config->clients, &client_fd->fd);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Could not find fd in connection table");
        return false;
    }
    struct t_client_data *data = (struct t_client_data *)node->data;

    if (client_fd->revents & POLLHUP) {
        MYGPIOD_LOG_INFO("Client#%u: Connection closed", node->id);
        list_remove_node(&config->clients, node);
        server_free_free_client_connection(node);
        return true;
    }

    switch(data->state) {
        case CLIENT_SOCKET_STATE_READING: {
            size_t max_bytes = sizeof(data->buf_in) - (size_t)data->bytes_in - 1;
            ssize_t result = read(data->fd, data->buf_in + data->bytes_in, max_bytes);
            if (result < 0) {
                MYGPIOD_LOG_ERROR("Client#%u: Could not read from socket", node->id);
                return false;
            }
            data->bytes_in += result;
            char *buf_end = memchr(data->buf_in, '\n', (size_t)data->bytes_in);
            if (buf_end) {
                chomp(data->buf_in, (size_t)data->bytes_in);
                MYGPIOD_LOG_INFO("Client#%u: Got command \"%s\"", node->id, data->buf_in);
                data->state = CLIENT_SOCKET_STATE_WRITING;
                data->bytes_in = 0;
                data->events = POLLOUT;
            }
            return true;
        }
        case CLIENT_SOCKET_STATE_WRITING: {
            size_t max_bytes = strlen(data->buf_out) - (size_t)data->bytes_out;
            ssize_t result = write(data->fd, data->buf_out + data->bytes_out, max_bytes);
            if (result < 0) {
                MYGPIOD_LOG_ERROR("Client#%u: Could not write to socket", node->id);
                return -1;
            }
            data->bytes_out += result;
            if ((size_t)result == max_bytes) {
                data->state = CLIENT_SOCKET_STATE_READING;
                data->events = POLLIN;
            }
            return true;
        }
    }
    return false;
}

// private functions

/**
 * Gets the gpio in node by timerfd
 * @param gpios_in list of in gpios
 * @param fd timer_fd
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

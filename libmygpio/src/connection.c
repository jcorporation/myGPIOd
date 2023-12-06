/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/include/libmygpio.h"
#include "libmygpio/src/connection.h"

#include "libmygpio/src/protocol.h"
#include "libmygpio/src/socket.h"
#include "libmygpio/src/util.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// privat definitions

static bool check_response_version(struct t_mygpio_connection *connection);
static bool parse_version(const char *str, struct t_mygpio_connection *connection);

// public functions

/**
 * Creates a new connection and checks the initial server response
 * @param socket_path unix socket path to connect
 * @return allocated connection struct
 */
struct t_mygpio_connection *mygpio_connection_new(const char *socket_path) {
    struct t_mygpio_connection *connection = malloc(sizeof(struct t_mygpio_connection));
    if (connection == NULL) {
        return NULL;
    }
    buf_init(&connection->buf_in);
    connection->version_string = NULL;
    connection->fd = socket_connect(socket_path);
    if (connection->fd == -1) {
        mygpio_connection_free(connection);
        return NULL;
    }
    if (socket_recv_line(connection->fd, &connection->buf_in) == false ||
        check_response_ok(connection->buf_in.buffer) == false ||
        socket_recv_line(connection->fd, &connection->buf_in) == false ||
        check_response_version(connection) == false ||
        socket_recv_line(connection->fd, &connection->buf_in) == false ||
        check_response_end(connection->buf_in.buffer) == false)
    {
        mygpio_connection_free(connection);
        return NULL;
    }
    return connection;
}

/**
 * Closes and frees the connection
 * @param connection connection struct
 */
void mygpio_connection_free(struct t_mygpio_connection *connection) {
    if (connection != NULL) {
        socket_close(connection->fd);
        buf_clear(&connection->buf_in);
        if (connection->version_string != NULL) {
            free(connection->version_string);
        }
        free(connection);
    }
}

/**
 * Returns the connection version string
 * @param connection connection struct
 * @return the myGPIOd version
 */
const char *mygpio_connection_version(struct t_mygpio_connection *connection) {
    return connection->version_string;
}

// private functions

/**
 * Checks for a correct version response line
 * @param connection connection struct
 * @return true on success, else false
 */
static bool check_response_version(struct t_mygpio_connection *connection) {
    struct t_mygpio_pair *pair = parse_pair(connection->buf_in.buffer);
    if (pair == NULL) {
        return false;
    }
    if (strcmp(pair->name, "version") != 0 ||
        parse_version(pair->value, connection) == false)
    {
        free_pair(pair);
        return false;
    }
    connection->version_string = strdup(pair->value);
    return true;
}

/**
 * Parses the myGPIOd version string to major.minor.patch
 * @param str string to parse
 * @param connection connection to populate the version
 * @return true on success, else false
 */
static bool parse_version(const char *str, struct t_mygpio_connection *connection) {
    char *rest;
    if (parse_int(str, &connection->version_major, &rest, 0, 99) == false) {
        return false;
    }
    if (*rest != '.') { return false; }
    rest++;
    if (parse_int(rest, &connection->version_minor, &rest, 0, 99) == false) {
        return false;
    }
    if (*rest != '.') { return false; }
    rest++;
    if (parse_int(rest, &connection->version_patch, &rest, 0, 99) == false) {
        return false;
    }
    if (*rest != '\0') { return false; }
    return true;
}

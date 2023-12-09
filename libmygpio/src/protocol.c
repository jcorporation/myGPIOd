/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/src/protocol.h"

#include "libmygpio/src/connection.h"
#include "libmygpio/src/pair.h"
#include "libmygpio/src/socket.h"
#include "libmygpio/src/util.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// private definitions

static bool parse_version(const char *str, struct t_mygpio_connection *connection);

// public functions

/**
 * Writes the format string to the output buffer and writes it to the socket.
 * @param connection connection struct
 * @param fmt format string
 * @param ... variadic arguments for the format string
 * @return true on success, else false
 */
bool send_line(struct t_mygpio_connection *connection, const char *fmt, ...) {
    //TODO: resize buffer if it is to small.
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(connection->buf_out.buffer, connection->buf_out.capacity, fmt, args);
    va_end(args);
    if (written <= 0 ||
        written >= (int)connection->buf_out.capacity)
    {
        buf_clear(&connection->buf_out);
        connection_set_state(connection, MYGPIO_STATE_ERROR, "Buffer write error");
        return false;
    }
    connection->buf_out.len = (size_t)written;
    bool rc = socket_send_line(connection->fd, &connection->buf_out);
    if (rc == false) {
        connection_set_state(connection, MYGPIO_STATE_ERROR, "Socket write error");
    }
    return rc;
}

/**
 * Checks the response status. Populates the connection error buffer
 * @param connection connection struct
 * @return true on success, else false
 */
bool recv_response_status(struct t_mygpio_connection *connection) {
    if (socket_recv_line(connection->fd, &connection->buf_in, connection->timeout) == false) {
        return false;
    }
    if (strcmp(connection->buf_in.buffer, "OK") == 0) {
        return true;
    }
    if (connection->error != NULL) {
        free(connection->error);
        connection->error = NULL;
    }
    if (strncmp(connection->buf_in.buffer, "ERROR:", 6) == 0) {
        char *p = strchr(connection->buf_in.buffer, ':');
        p++;
        connection_set_state(connection, MYGPIO_STATE_ERROR, p);
    }
    else {
        connection_set_state(connection, MYGPIO_STATE_ERROR, "Malformed server response");
    }
    return false;
}

/**
 * Receives and parses the version from myGPIOd connect handshake
 * @param connection connection struct
 * @return true on success, else false
 */
bool recv_version(struct t_mygpio_connection *connection) {
    struct t_mygpio_pair *pair = mygpio_recv_pair(connection);
    if (pair == NULL) {
        return false;
    }
    if (strcmp(pair->name, "version") != 0 ||
        parse_version(pair->value, connection) == false)
    {
        mygpio_free_pair(pair);
        return false;
    }
    mygpio_free_pair(pair);
    return true;
}

/**
 * Finish reading the server response
 * @param connection connection struct
 * @return true on success, else false
 */
bool mygpio_response_end(struct t_mygpio_connection *connection) {
    while (strcmp(connection->buf_in.buffer, "END") != 0) {
        if (socket_recv_line(connection->fd, &connection->buf_in, 0) == false) {
            connection_set_state(connection, MYGPIO_STATE_ERROR, "Reading response failed");
            return false;
        }
    }
    return true;
}

// private functions

/**
 * Parses the myGPIOd version string to major.minor.patch
 * @param str string to parse
 * @param connection connection to populate the version
 * @return true on success, else false
 */
static bool parse_version(const char *str, struct t_mygpio_connection *connection) {
    char *rest;
    if (parse_uint(str, &connection->version[0], &rest, 0, 99) == false) {
        return false;
    }
    if (*rest != '.') { return false; }
    rest++;
    if (parse_uint(rest, &connection->version[1], &rest, 0, 99) == false) {
        return false;
    }
    if (*rest != '.') { return false; }
    rest++;
    if (parse_uint(rest, &connection->version[2], &rest, 0, 99) == false) {
        return false;
    }
    if (*rest != '\0') { return false; }
    return true;
}

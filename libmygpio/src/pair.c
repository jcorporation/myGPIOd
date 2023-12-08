/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/include/libmygpio/pair.h"
#include "libmygpio/src/connection.h"
#include "libmygpio/src/socket.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// private definitions

static struct t_mygpio_pair *mygpio_parse_pair(const char *line);

// public functions

/**
 * Receives and parses the  key/value pair from the response
 * @param connection connection struct
 * @return the pair or NULL on error or response end
 */
struct t_mygpio_pair *mygpio_recv_pair(struct t_mygpio_connection *connection) {
    socket_recv_line(connection->fd, &connection->buf_in);
    if (connection->buf_in.len == 0) {
        return NULL;
    }
    if (strcmp(connection->buf_in.buffer, "END") == 0) {
        return NULL;
    }
    return mygpio_parse_pair(connection->buf_in.buffer);
}

/**
 * Frees the key/value pair
 * @param pair pair to free
 */
void mygpio_free_pair(struct t_mygpio_pair *pair) {
    pair->name = NULL;
    pair->value = NULL;
    free(pair);
}

// private functions

/**
 * Parses a line to a key/value pair.
 * Key and value are only pointers and are not copied.
 * @param line line to parse
 * @return allocated pair or NULL on error
 */
struct t_mygpio_pair *mygpio_parse_pair(const char *line) {
    struct t_mygpio_pair *pair = malloc(sizeof(struct t_mygpio_pair));
    if (pair == NULL) {
        return NULL;
    }
    char *p = strchr(line, ':');
    if (p == NULL) {
        mygpio_free_pair(pair);
        return NULL;
    }
    pair->name = line;
    *p = '\0';
    pair->value = p + 1;
    return pair;
}

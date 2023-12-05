/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/include/libmygpio.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct t_mygpio_connection {
    int fd;
    const char *socket_path;
};

struct t_mygpio_connection *mygpio_connection_new(const char *socket_path) {
    struct t_mygpio_connection *connection = malloc(sizeof(struct t_mygpio_connection));
    if (connection == NULL) {
        return NULL;
    }
    printf("%s", socket_path);
    return connection;
}

void mygpio_connection_free(struct t_mygpio_connection *connection) {
    if (connection != NULL) {
        close(connection->fd);
        free(connection);
    }
}

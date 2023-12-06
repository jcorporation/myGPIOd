/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/src/socket.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/**
 * Connects to a socket
 * @param socket_path unix socket to connect
 * @return open file descriptor
 */
int socket_connect(const char *socket_path) {
    struct sockaddr_un address = { 0 };
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socket_path, 108);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        return -1;
    }

    int flags = fcntl(fd, F_GETFD, 0);
    if (fcntl(fd, F_SETFD, flags | O_NONBLOCK | O_CLOEXEC)) {
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        close(fd);
        return -1;
    }
    return fd;
}

/**
 * Closes the file descriptor
 * @param fd file descriptor
 */
void socket_close(int fd) {
    if (fd > 0) {
        close(fd);
    }
}

/**
 * Receives a line from the socket and crops the ending LF.
 * @param fd file descriptor to read
 * @param buf buffer to fill
 * @return true on success, else false
 */
bool socket_recv_line(int fd, struct t_buf *buf) {
    buf_reset(buf);
    ssize_t nread;
    while ((nread = read(fd, buf->buffer + buf->len, 1)) > 0) {
        buf->len += nread;
        if (buf->len == buf->capacity) {
            buf->capacity += BUFFER_SIZE_INIT;
            if (buf->capacity > BUFFER_SIZE_MAX) {
                return false;
            }
            char *new_buf = realloc(buf->buffer, buf->capacity);
            assert(new_buf);
            buf->buffer = new_buf;
            assert(buf->buffer);
        }
        buf->buffer[buf->len] = '\0';
        if (buf->buffer[buf->len - 1] == '\n') {
            buf->buffer[buf->len - 1] = '\0';
            buf->len--;
            return true;
        }
    }
    return false;
}

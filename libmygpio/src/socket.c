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
    if (fcntl(fd, F_SETFD, flags | O_CLOEXEC)) {
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
 * This command blocks.
 * @param fd file descriptor to read
 * @param buf buffer to fill
 * @param wait wait for input?
 * @return true on success, else false
 */
bool socket_recv_line(int fd, struct t_buf *buf, bool wait) {
    buf_reset(buf);
    ssize_t nread;
    int flag = wait == true
        ? 0
        : MSG_DONTWAIT;
    while ((nread = recv(fd, buf->buffer + buf->len, 1, flag)) > 0) {
        buf->len += nread;
        if (buf->len == buf->capacity) {
            buf->capacity += BUFFER_SIZE_INIT;
            if (buf->capacity > BUFFER_SIZE_MAX) {
                return false;
            }
            char *new_buf = realloc(buf->buffer, buf->capacity);
            assert(new_buf);
            buf->buffer = new_buf;
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

/**
 * Writes a line to the socket.
 * Buffer will be cleared.
 * @param fd socket to write
 * @param buf buffer to write
 * @return true on success, else false
 */
bool socket_send_line(int fd, struct t_buf *buf) {
    ssize_t nwrite;
    size_t written = 0;
    size_t max_bytes = buf->len;
    while ((nwrite = write(fd, buf->buffer + written, max_bytes)) > 0) {
        if (nwrite < 0) {
            buf_reset(buf);
            return false;
        }
        written += (size_t)nwrite;
        max_bytes = buf->len - written;
        if (written == buf->len) {
            buf_reset(buf);
            if (write(fd, "\n", 1) != 1) {
                return false;
            }
            return true;
        }
    }
    buf_reset(buf);
    return false;
}

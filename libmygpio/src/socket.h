/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_SOCKET_H
#define LIBMYGPIO_SOCKET_H

#include "libmygpio/src/buffer.h"

#include <stdbool.h>

int socket_connect(const char *socket_path);
void socket_close(int fd);
bool socket_recv_line(int fd, struct t_buf *buf);

#endif

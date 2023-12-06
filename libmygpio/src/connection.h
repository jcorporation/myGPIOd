/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_CONNECTION_H
#define LIBMYGPIO_CONNECTION_H

#include "libmygpio/src/buffer.h"

struct t_mygpio_connection {
    int fd;
    char *socket_path;
    struct t_buf buf_in;
    char *version_string;
    int version_major;
    int version_minor;
    int version_patch;
};

#endif

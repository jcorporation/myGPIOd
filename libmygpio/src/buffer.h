/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_SRC_BUFFER_H
#define LIBMYGPIO_SRC_BUFFER_H

#include <stddef.h>

#define BUFFER_SIZE_INIT 256
#define BUFFER_SIZE_MAX 2048

struct t_buf {
    char *buffer;
    size_t len;
    size_t capacity;
};

void libmygpio_buf_init(struct t_buf *buf);
void libmygpio_buf_reset(struct t_buf *buf);
void libmygpio_buf_clear(struct t_buf *buf);

#endif

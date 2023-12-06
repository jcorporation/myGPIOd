/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/src/buffer.h"

#include <assert.h>
#include <stdlib.h>

/**
 * Initializes the buffer struct
 * @param buf 
 */
void buf_init(struct t_buf *buf) {
    buf->buffer = malloc(BUFFER_SIZE_INIT);
    assert(buf->buffer);
    buf->buffer[0] = '\0';
    buf->len = 0;
    buf->capacity = BUFFER_SIZE_INIT;
}

/**
 * Resets the buffer struct
 * @param buf 
 */
void buf_reset(struct t_buf *buf) {
    buf->buffer[0] = '\0';
    buf->len = 0;
}

/**
 * Clears the buffer struct
 * @param buf 
 */
void buf_clear(struct t_buf *buf) {
    free(buf->buffer);
}

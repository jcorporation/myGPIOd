/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "libmygpio/include/libmygpio/libmygpio_protocol.h"
#include "libmygpio/include/libmygpio/libmygpio_raspberry_vcio.h"
#include "libmygpio/src/pair.h"
#include "libmygpio/src/protocol.h"

#include <string.h>

/**
 * Returns the current value reported by /dev/vcio
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param command On of: vciotemp, vciovolts, vcioclock, vciothrottled
 * @return Current value or NULL on error
 */
struct t_mygpio_pair *mygpio_vcio(struct t_mygpio_connection *connection, const char *command) {
    struct t_mygpio_pair *pair = NULL;
    if (libmygpio_send_line(connection, command) == false ||
        libmygpio_recv_response_status(connection) == false ||
        (pair = mygpio_recv_pair(connection)) == NULL ||
        strcmp(pair->name, "value") != 0)
    {
        mygpio_free_pair(pair);
        return NULL;
    }
    return pair;
}

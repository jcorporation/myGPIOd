/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "libmygpio/include/libmygpio/libmygpio_hook.h"
#include "libmygpio/include/libmygpio/libmygpio_protocol.h"
#include "libmygpio/src/protocol.h"

/**
 * Triggers a hook
 * @param connection connection struct
 * @param name Hook name
 * @return true on success, else false
 */
bool mygpio_hook(struct t_mygpio_connection *connection, const char *name) {
    return libmygpio_send_line(connection, "hook %s", name) &&
        libmygpio_recv_response_status(connection) &&
        mygpio_response_end(connection);
}

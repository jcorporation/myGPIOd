/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/src/idle.h"

#include "libmygpio/src/protocol.h"

#include <poll.h>

/**
 * Sends the idle command
 * @param connection connection struct
 * @return true on success, else false
 */
bool mygpio_send_idle(struct t_mygpio_connection *connection) {
    return send_line(connection, "idle");
}

/**
 * Waits for an idle event
 * @param connection connection struct
 * @param timeout timeout in ms, use -1 to wait without a timeout
 * @return true if events are waiting, false on timeout
 */
bool mygpio_recv_idle(struct t_mygpio_connection *connection, int timeout) {
    struct pollfd pfds[1];
    pfds[0].fd = mygpio_connection_get_fd(connection);
    pfds[0].events = POLLIN;
    int events = poll(pfds, 1, timeout);
    if (events > 0) {
        return recv_response_status(connection);
    }
    return false;
}

/**
 * Sends the noidle command
 * @param connection connection struct
 * @return true on success, else false
 */
bool mygpio_send_noidle(struct t_mygpio_connection *connection) {
    return send_line(connection, "noidle") &&
        recv_response_status(connection);
}

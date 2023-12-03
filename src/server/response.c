/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "src/server/response.h"

/**
 * Starts a new response by clearing the output buffer.
 * It sends the response from buf_out to the client.
 * @param data pointer to client data
 */
void server_response_start(struct t_client_data *data) {
    sdsclear(data->buf_out);
}

/**
 * Appends the message to the output buffer
 * @param data pointer to client data
 * @param message message to append
 */
void server_response_append(struct t_client_data *data, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    data->buf_out = sdscatvprintf(data->buf_out, fmt, args);
    va_end(args);
}

/**
 * Sets the client state to writing.
 * It sends the response from buf_out to the client.
 * @param data pointer to client data
 */
void server_response_end(struct t_client_data *data) {
    data->state = CLIENT_SOCKET_STATE_WRITING;
    data->bytes_out = 0;
    data->events = POLLOUT;
}

/**
 * Shortcut for server_response_start, server_response_append and server_response_end.
 * @param data pointer to client data
 * @param message message to send
 */
void server_response_send(struct t_client_data *data, const char *message) {
    server_response_start(data);
    server_response_append(data, "%s", message);
    server_response_end(data);
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "mygpiod/server/response.h"

/**
 * Starts a new response by clearing the output buffer.
 * It sends the response from buf_out to the client.
 * @param data pointer to client data
 */
void server_response_start(struct t_client_data *client_data) {
    sdsclear(client_data->buf_out);
}

/**
 * Appends the message to the output buffer
 * @param client_data pointer to client data
 * @param fmt printf style format string
 * @param ... variadic arguments for the format string
 */
void server_response_append(struct t_client_data *client_data, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    client_data->buf_out = sdscatvprintf(client_data->buf_out, fmt, args);
    va_end(args);
}

/**
 * Sets the client state to writing.
 * It sends the response from buf_out to the client.
 * @param client_data pointer to client data
 */
void server_response_end(struct t_client_data *client_data) {
    client_data->state = CLIENT_SOCKET_STATE_WRITING;
    client_data->bytes_out = 0;
    client_data->events = POLLOUT;
}

/**
 * Shortcut for server_response_start, server_response_append and server_response_end.
 * @param client_data pointer to client data
 * @param message message to send
 */
void server_response_send(struct t_client_data *client_data, const char *message) {
    server_response_start(client_data);
    server_response_append(client_data, "%s", message);
    server_response_end(client_data);
}

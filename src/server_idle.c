/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "server_idle.h"

#include "server_protocol.h"
#include "server_socket.h"

#include <stdio.h>
#include <stdlib.h>

// private definitions

bool send_idle_events(struct t_list_node *node);

// public functions

/**
 * Enters the idle mode.
 * In the idle mode only the "noidle" command is allowed.
 * Events are sent as soon they occurs.
 * @param node list node holding the client data
 * @return true on success, else false
 */
bool handle_idle(struct t_list_node *node) {
    struct t_client_data *data = (struct t_client_data *)node->data;
    if (data->waiting_events.length == 0) {
        data->state = CLIENT_SOCKET_STATE_IDLE;
        return true;
    }
    return send_idle_events(node->data);
}

/**
 * Exits the idle mode and sends the waiting idle events to the client
 * @param node list node holding the client data
 * @return true on success, else false
 */
bool handle_noidle(struct t_list_node *node) {
    struct t_client_data *data = (struct t_client_data *)node->data;
    if (data->waiting_events.length == 0) {
        server_send_response(node, DEFAULT_OK_MSG_PREFIX);
        data->state = CLIENT_SOCKET_STATE_WRITING;
        return true;
    }
    return send_idle_events(node->data);
}

// private functions

/**
 * Sends the waiting idle events to the client
 * @param node list node holding the client data
 * @return true on success, else false
 */
bool send_idle_events(struct t_list_node *node) {
    struct t_client_data *data = (struct t_client_data *)node->data;

    char *response = malloc(SERVER_OUTPUT_BUFFER_SIZE);
    size_t len = snprintf(response, SERVER_OUTPUT_BUFFER_SIZE, DEFAULT_OK_MSG_PREFIX);
    struct t_list_node *current = data->waiting_events.head;
    while (current != NULL) {
        len = snprintf(response + len, SERVER_OUTPUT_BUFFER_SIZE - len, "event:\n");
        current = current -> next;
    }
    list_clear(&data->waiting_events, NULL);

    data->state = CLIENT_SOCKET_STATE_WRITING;
    server_send_response(node, response);
    free(response);
    return true;
}

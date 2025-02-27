/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server/idle.h"

#include "mygpiod/lib/events.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/server/response.h"
#include "mygpiod/server/socket.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Enters the idle mode and disabled the timeout.
 * In the idle mode only the "noidle" command is allowed.
 * Events are sent as soon they occurs.
 * @param client_node list node holding the client data
 * @return true on success, else false
 */
bool handle_idle(struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    if (client_data->waiting_events.length == 0) {
        server_client_connection_remove_timeout(client_data);
        MYGPIOD_LOG_INFO("Client#%u: Entering idle mode", client_node->id);
        client_data->state = CLIENT_SOCKET_STATE_IDLE;
        return true;
    }
    return send_idle_events(client_node, false);
}

/**
 * Exits the idle mode and sends the waiting idle events to the client
 * @param client_node list node holding the client data
 * @return true on success, else false
 */
bool handle_noidle(struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    MYGPIOD_LOG_INFO("Client#%u: Leaving idle mode", client_node->id);
    client_data->timeout_fd = server_client_connection_set_timeout(client_data->timeout_fd, config->socket_timeout_s);
    if (client_data->waiting_events.length == 0) {
        server_response_send(client_data, DEFAULT_MSG_OK "\n" DEFAULT_MSG_END);
        return true;
    }
    return send_idle_events(client_node, true);
}

/**
 * Sends the waiting idle events to the client
 * @param client_node list node holding the client data
 * @return true on success, else false
 */
bool send_idle_events(struct t_list_node *client_node, bool send_ok) {
    MYGPIOD_LOG_INFO("Client#%u: Sending idle events", client_node->id);
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;

    server_response_start(client_data);
    if (send_ok == true) {
        server_response_append(client_data, "%s", DEFAULT_MSG_OK);
    }
    struct t_list_node *current = client_data->waiting_events.head;
    while (current != NULL) {
        struct t_event_data *event_data = (struct t_event_data *)current->data;
        server_response_append(client_data, "gpio:%u", current->id);
        server_response_append(client_data, "event:%s", mygpiod_event_name(event_data->mygpiod_event_type));
        server_response_append(client_data, "timestamp_ms:%llu", (long long unsigned)(event_data->timestamp / 1000000));
        current = current -> next;
    }
    list_clear(&client_data->waiting_events, event_data_clear);
    server_response_append(client_data, "%s", DEFAULT_MSG_END);
    server_response_end(client_data);
    return true;
}

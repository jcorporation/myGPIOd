/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Idle command handling
 */

#include "compile_time.h"
#include "mygpiod/server_socket/idle.h"

#include "mygpiod/input/event_code.h"
#include "mygpiod/input/event_type.h"
#include "mygpiod/lib/event_types.h"
#include "mygpiod/lib/events.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/server_socket/response.h"
#include "mygpiod/server_socket/socket.h"

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
 * @param config Pointer to config
 * @param client_node List node holding the client data
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
 * @param client_node List node holding the client data
 * @param send_ok Send OK before response
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
        server_response_append(client_data, "event:%s", mygpiod_event_name(event_data->mygpiod_event_type));
        server_response_append(client_data, "timestamp_ms:%llu", (long long unsigned)(event_data->timestamp_ns / 1000000));
        if (event_data->mygpiod_event_type == MYGPIOD_EVENT_INPUT) {
            server_response_append(client_data, "device:%s", event_data->input_event.device->name);
            server_response_append(client_data, "type:%s", input_event_type_name(event_data->input_event.data.type));
            server_response_append(client_data, "code:%s", input_event_code_name(event_data->input_event.data.type, event_data->input_event.data.code));
            server_response_append(client_data, "value:%u", event_data->input_event.data.value);
        }
        else {
            server_response_append(client_data, "gpio:%u", current->id);
        }
        current = current -> next;
    }
    list_clear(&client_data->waiting_events, event_data_clear);
    server_response_append(client_data, "%s", DEFAULT_MSG_END);
    server_response_end(client_data);
    return true;
}

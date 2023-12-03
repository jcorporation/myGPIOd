/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "src/server/idle.h"

#include "src/lib/events.h"
#include "src/lib/log.h"
#include "src/server/protocol.h"
#include "src/server/response.h"
#include "src/server/socket.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Enters the idle mode and disabled the timeout.
 * In the idle mode only the "noidle" command is allowed.
 * Events are sent as soon they occurs.
 * @param node list node holding the client data
 * @return true on success, else false
 */
bool handle_idle(struct t_list_node *node) {
    struct t_client_data *data = (struct t_client_data *)node->data;
    if (data->waiting_events.length == 0) {
        server_client_connection_remove_timeout(data);
        MYGPIOD_LOG_INFO("Client#%u: Entering idle mode", node->id);
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
bool handle_noidle(struct t_config *config, struct t_list_node *node) {
    struct t_client_data *data = (struct t_client_data *)node->data;
    MYGPIOD_LOG_INFO("Client#%u: Leaving idle mode", node->id);
    data->timeout_fd = server_client_connection_set_timeout(data->timeout_fd, config->socket_timeout);
    if (data->waiting_events.length == 0) {
        server_response_send(data, DEFAULT_OK_MSG_PREFIX DEFAULT_END_MSG);
        return true;
    }
    return send_idle_events(node->data);
}

/**
 * Sends the waiting idle events to the client
 * @param node list node holding the client data
 * @return true on success, else false
 */
bool send_idle_events(struct t_list_node *node) {
    MYGPIOD_LOG_DEBUG("Client#%u: Sending idle events", node->id);
    struct t_client_data *data = (struct t_client_data *)node->data;

    server_response_start(data);
    server_response_append(data, "%s", DEFAULT_OK_MSG_PREFIX);
    struct t_list_node *current = data->waiting_events.head;
    while (current != NULL) {
        struct t_event_data *event_data = (struct t_event_data *)current->data;
        server_response_append(data, "gpio:%u\n", current->id);
        server_response_append(data, "event:%s\n", mygpiod_event_name(event_data->mygpiod_event_type));
        server_response_append(data, "time:%lld.%ld\n", (long long)event_data->ts.tv_sec, (event_data->ts.tv_nsec / 1000000));
        current = current -> next;
    }
    list_clear(&data->waiting_events, event_data_clear);
    server_response_append(data, "%s", DEFAULT_END_MSG);
    server_response_end(data);
    return true;
}

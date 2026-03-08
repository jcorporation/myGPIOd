/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Hook command handling
 */

#include "compile_time.h"
#include "mygpiod/server_socket/timerev.h"

#include "mygpiod/config/timer_ev.h"
#include "mygpiod/lib/timer.h"
#include "mygpiod/server_socket/response.h"

/**
 * Hook command handler.
  * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_timerevlist(struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    server_response_start(client_data);
    server_response_append(client_data, "%s", DEFAULT_MSG_OK);
    struct t_list_node *current = config->timer_definitions.head;
    while (current != NULL) {
        struct t_timer_definition *data = (struct t_timer_definition *)current->data;
        server_response_append(client_data, "name:%s", data->name);
        server_response_append(client_data, "next:%lld", (long long)timer_get_next_expire_ts(data->name, data->fd));
        current = current->next;
    }
    server_response_append(client_data, "%s", DEFAULT_MSG_END);
    server_response_end(client_data);
    return true;
}

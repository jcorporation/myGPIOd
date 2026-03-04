/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Hook command handling
 */

#include "compile_time.h"
#include "mygpiod/server_socket/hook.h"

#include "mygpiod/hook/action.h"
#include "mygpiod/server_socket/response.h"

/**
 * Hook command handler.
 * @param options client command
 * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_hook(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    if (options->len != 2) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid number of arguments");
        return false;
    }

    if (hook_action_handler(config, options->args[1]) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid hook");
        return false;
    }

    server_response_send(client_data, DEFAULT_MSG_OK "\n" DEFAULT_MSG_END);
    return true;
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server/event.h"

#include "mygpio-common/util.h"
#include "mygpiod/gpio/action.h"
#include "mygpiod/lib/util.h"
#include "mygpiod/server/response.h"
#include "mygpiod/server/socket.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * Emmits a syntetic event calling action_handle.
 * This protocol command is only enabled in debug builds.
 * @param options client command
 * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_event(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    if (options->len != 3) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid number of arguments");
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(options->args[1], &gpio, NULL, 0, GPIOS_MAX) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid GPIO number");
        return false;
    }
    errno = 0;
    enum gpiod_edge_event_type event_type = parse_event_type(options->args[2]);
    if (errno != 0 ) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid event type");
        return false;
    }

    struct t_list_node *node = list_node_by_id(&config->gpios_in, gpio);
    if (node == NULL) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "GPIO not configured");
        return false;
    }
    struct t_gpio_in_data *data = node->data;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t timestamp = (uint64_t)(ts.tv_sec * 1000000000 + ts.tv_nsec);

    action_handle(config, gpio, timestamp, event_type, data);
    server_response_send(client_data, DEFAULT_MSG_OK "\n" DEFAULT_MSG_END);
    return true;
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Raspberry command handling
 */

#include "compile_time.h"
#include "mygpiod/server_socket/raspberry.h"

#include "mygpiod/lib/sds_extras.h"
#include "mygpiod/raspberry/vcgencmd.h"
#include "mygpiod/server_socket/response.h"

/**
 * Handles the gpioget command
 * @param client_node client
 * @param command Command to submit
 * @return true on success, else false
 */
bool handle_raspberry_vcio(struct t_list_node *client_node, const char *command) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;

    bool rc;
    sds result = vcgencmd(command, sdsempty(), &rc);
    if (rc == false) {
        sds error = sdscatfmt(sdsempty(), "%s%S", DEFAULT_MSG_ERROR, result);
        server_response_send(client_data, error);
        FREE_SDS(result);
        FREE_SDS(error);
        return false;
    }
    server_response_start(client_data);
    server_response_append(client_data, "%s", DEFAULT_MSG_OK);
    server_response_append(client_data, "value:%s", result);
    server_response_append(client_data, "%s", DEFAULT_MSG_END);
    server_response_end(client_data);
    FREE_SDS(result);
    return true;
}

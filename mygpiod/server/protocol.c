/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server/protocol.h"

#include "mygpiod/lib/log.h"
#include "mygpiod/lib/util.h"
#include "mygpiod/server/gpio.h"
#include "mygpiod/server/idle.h"
#include "mygpiod/server/response.h"
#include "mygpiod/server/socket.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

// private definitions

static const char *cmd_strs[] = { CMDS(GEN_STR) };

static enum cmd_ids parse_command(sds cmd);

// public functions

/**
 * Handles the client commands
 * @param config pointer to config
 * @param node pointer to node holding the client connection
 * @return true on success, else false
 */
bool server_protocol_handler(struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;

    int count = 0;
    sds *args = sdssplitargs(client_data->buf_in, &count);
    sdsclear(client_data->buf_in);
    if (count == 0) {
        sdsfreesplitres(args, count);
        return true;
    }
    enum cmd_ids cmd_id = parse_command(args[0]);
    if (client_data->state == CLIENT_SOCKET_STATE_IDLE &&
        cmd_id != CMD_NOIDLE)
    {
        MYGPIOD_LOG_ERROR("Client#%u: Only noidle command is allowed", client_node->id);
        server_response_send(client_data, DEFAULT_MSG_ERROR "In idle state, only the noidle command is allowed");
        sdsfreesplitres(args, count);
        return false;
    }

    struct t_cmd_options options = {
        .args = args,
        .len = count
    };

    MYGPIOD_LOG_INFO("Client#%u: Command: \"%s\"", client_node->id, get_cmd_name(cmd_id));
    bool rc = true;
    switch(cmd_id) {
        case CMD_CLOSE:
            rc = server_client_disconnect(&config->clients, client_node);
            break;
        case CMD_IDLE:
            rc = handle_idle(client_node);
            break;
        case CMD_NOIDLE:
            rc = handle_noidle(config, client_node);
            break;
        case CMD_GPIOGET:
            rc = handle_gpioget(&options, config, client_node);
            break;
        case CMD_GPIOSET:
            rc = handle_gpioset(&options, config, client_node);
            break;
        case CMD_GPIOLIST:
            rc = handle_gpiolist(config, client_node);
            break;
        case CMD_INVALID:
        case CMD_COUNT:
            MYGPIOD_LOG_ERROR("Client#%u: Invalid command", client_node->id);
            server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid command");
            rc = false;
    }
    sdsfreesplitres(args, count);
    return rc;
}

// private functions

/**
 * Parses the command
 * @param cmd line to parse
 * @return the cmd id
 */
static enum cmd_ids parse_command(sds cmd) {
    sds cmd_str = sdscatfmt(sdsempty(), "CMD_%s", cmd);
    sdstoupper(cmd_str);
    enum cmd_ids cmd_id = get_cmd_id(cmd_str);
    FREE_SDS(cmd_str);
    return cmd_id;
}

/**
 * Converts a string to the cmd_ids enum
 * @param cmd string to convert
 * @return enum mympd_cmd_ids
 */
enum cmd_ids get_cmd_id(const char *cmd) {
    for (unsigned i = 0; i < CMD_COUNT; i++) {
        if (strcmp(cmd, cmd_strs[i]) == 0) {
            return i;
        }
    }
    return CMD_INVALID;
}

/**
 * Converts the mympd_cmd_ids enum to the string
 * @param cmd_id myMPD API method
 * @return the API method as string
 */
const char *get_cmd_name(enum cmd_ids cmd_id) {
    if (cmd_id >= CMD_COUNT) {
        return NULL;
    }
    return cmd_strs[cmd_id];
}

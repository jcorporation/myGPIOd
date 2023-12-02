/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "server_protocol.h"

#include "log.h"
#include "server_idle.h"
#include "server_socket.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

// private definitions

#define WELCOME_MESSAGE "OK\nversion:myGPIOd " MYGPIOD_VERSION "\n"
#define MAX_CMD_LEN 25

static const char *cmd_strs[] = { CMDS(GEN_STR) };

static void parse_command(char *line, enum cmd_ids *cmd_id, char **options);

// public functions

/**
 * Handles the client commands
 * @param config pointer to config
 * @param node pointer to node holding the client connection
 * @return true on success, else false
 */
bool server_protocol_handler(struct t_config *config, struct t_list_node *node) {
    struct t_client_data *data = (struct t_client_data *)node->data;
    enum cmd_ids cmd_id;
    char *options = NULL;
    parse_command(data->buf_in, &cmd_id, &options);
    if (data->state == CLIENT_SOCKET_STATE_IDLE &&
        cmd_id != CMD_NOIDLE)
    {
        server_send_response(node, DEFAULT_ERROR_MSG_PREFIX "In idle state, only the noidle command is allowed\n");
        return false;
    }
    MYGPIOD_LOG_INFO("Command: \"%s\", options: \"%s\"", get_cmd_name(cmd_id), options);
    switch(cmd_id) {
        case CMD_CLOSE:
            server_client_disconnect(&config->clients, node);
            return true;
        case CMD_IDLE:
            return handle_idle(node);
        case CMD_NOIDLE:
            return handle_noidle(node);
        case CMD_INVALID:
        case CMD_COUNT:
            server_send_response(node, DEFAULT_ERROR_MSG_PREFIX "Invalid command\n");
            MYGPIOD_LOG_ERROR("Invalid command \"%s\"", data->buf_in);
    }
    return false;
}

// private functions

/**
 * Splits the command from options and lookups the cmd_id
 * @param line line to parse
 * @param cmd_id the cmd id
 * @param options pointer to options string
 */
static void parse_command(char *line, enum cmd_ids *cmd_id, char **options) {
    char cmd_str[MAX_CMD_LEN];
    snprintf(cmd_str, 5, "CMD_");
    char *p = line;
    size_t i = 4;
    for (; i < MAX_CMD_LEN && *p != '\0'; p++, i++) {
        if (isspace(*p)) {
            p++;
            break;
        }
        cmd_str[i] = *p;
    }
    cmd_str[i] = '\0';
    *options = p;
    *cmd_id = get_cmd_id(cmd_str);
}

/**
 * Converts a string to the cmd_ids enum
 * @param cmd string to convert
 * @return enum mympd_cmd_ids
 */
enum cmd_ids get_cmd_id(const char *cmd) {
    for (unsigned i = 0; i < CMD_COUNT; i++) {
        if (strcasecmp(cmd, cmd_strs[i]) == 0) {
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

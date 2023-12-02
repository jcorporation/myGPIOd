/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_PROTOCOL_H
#define MYGPIOD_SERVER_PROTOCOL_H

#include "src/lib/config.h"

#include <stdbool.h>

// Socket server default responses
#define DEFAULT_ERROR_MSG_PREFIX "ERROR\nmessage:"
#define DEFAULT_OK_MSG_PREFIX "OK\n"

#define CMDS(X) \
    X(CMD_CLOSE) \
    X(CMD_IDLE) \
    X(CMD_INVALID) \
    X(CMD_NOIDLE) \
    X(CMD_COUNT)

/**
 * Helper macros
 */
#define GEN_ENUM(X) X,
#define GEN_STR(X) #X,

/**
 * Enum of commands
 */
enum cmd_ids {
    CMDS(GEN_ENUM)
};

bool server_protocol_handler(struct t_config *config, struct t_list_node *node);
enum cmd_ids get_cmd_id(const char *cmd);
const char *get_cmd_name(enum cmd_ids cmd_id);

#endif

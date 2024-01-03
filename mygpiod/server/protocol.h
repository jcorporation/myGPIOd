/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_PROTOCOL_H
#define MYGPIOD_SERVER_PROTOCOL_H

#include "mygpiod/lib/config.h"

#include <stdbool.h>

#define CMDS(X) \
    X(CMD_INVALID) \
    X(CMD_CLOSE) \
    X(CMD_IDLE) \
    X(CMD_NOIDLE) \
    X(CMD_GPIOLIST) \
    X(CMD_GPIOGET) \
    X(CMD_GPIOSET) \
    X(CMD_GPIOTOGGLE) \
    X(CMD_GPIOBLINK) \
    X(CMD_GPIOINFO) \
    X(CMD_EVENT) \
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

/**
 * Command and its arguments
 */
struct t_cmd_options {
    sds *args;  //!< sds array of command + arguments
    int len;    //!< length auf args array
};

bool server_protocol_handler(struct t_config *config, struct t_list_node *client_node);
enum cmd_ids get_cmd_id(const char *cmd);
const char *get_cmd_name(enum cmd_ids cmd_id);

#endif

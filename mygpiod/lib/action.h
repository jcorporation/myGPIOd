/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_LIB_ACTION_H
#define MYGPIOD_LIB_ACTION_H

#include "compile_time.h"
#include "dist/sds/sds.h"
#include "mygpiod/lib/list.h"

/**
 * Action types
 */
enum mygpiod_actions {
    MYGPIOD_ACTION_UNKNOWN = -1,  //!< Unknown action type
    MYGPIOD_ACTION_SYSTEM,        //!< System action
    MYGPIOD_ACTION_GPIO_SET,      //!< Set a GPIO value
    MYGPIOD_ACTION_GPIO_TOGGLE,   //!< Toggle a GPIO value
    MYGPIOD_ACTION_GPIO_BLINK,    //!< Blink a GPIO value
    #ifdef MYGPIOD_ENABLE_ACTION_MPC
        MYGPIOD_ACTION_MPC,       //!< Controls MPD
    #endif
    #ifdef MYGPIOD_ENABLE_ACTION_HTTP
        MYGPIOD_ACTION_HTTP,      //!< Makes an HTTP call
        MYGPIOD_ACTION_MYMPD,     //!< Executes a myMPD script
    #endif
};

/**
 * Struct holding the action for an event
 */
struct t_action {
    enum mygpiod_actions action;  //!< Action type
    sds option;                   //!< option for the action type
};

struct t_action *action_node_data_new(enum mygpiod_actions action, sds option);
void node_data_action_clear(struct t_list_node *node);
const char *lookup_action(enum mygpiod_actions action);
enum mygpiod_actions parse_action(const char *str);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device configuration
 */

#ifndef MYGPIOD_CONFIG_INPUT_EV_H
#define MYGPIOD_CONFIG_INPUT_EV_H

#include "mygpiod/actions/actions.h"

#include <stdbool.h>

/**
 * How to match an input event action
 */
enum input_event_match {
    MATCH_ALL,
    MATCH_VALUE,
};

/**
 * Config data for input event actions
 */
struct t_input_event_actions {
    unsigned short type;                 //!< Input event type
    enum input_event_match code_match;   //!< Matching for code
    unsigned short code;                 //!< Input event code
    enum input_event_match value_match;  //!< Matching for input event value
    unsigned int value;                  //!< Expected input event value
    unsigned state;                      //!< Current input event value, initial: 0
    struct t_action action;              //!< Action
};

/**
 * Config data for inputs
 */
struct t_input_device {
    sds name;                      //!< Device name /dev/input/...
    int fd;                        //!< File descriptor
    struct t_list event_actions;   //!< List of events
};

bool parse_input_ev(struct t_list *input_devices, sds config_value);
void input_data_clear(struct t_input_device *device);
void input_node_data_clear(struct t_list_node *node);

#endif

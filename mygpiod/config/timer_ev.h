/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Timer event configuration
 */

#ifndef MYGPIOD_CONFIG_TIMER_EV_H
#define MYGPIOD_CONFIG_TIMER_EV_H

#include "mygpiod/actions/actions.h"

#include <stdbool.h>


/**
 * Config data for inputs
 */
struct t_timer_definition {
    sds name;                      //!< Timer event name
    int fd;                        //!< File descriptor
    int start_hour;                //!< Start hour
    int start_minute;              //!< Start minute
    int interval;                  //!< Interval
    bool weekdays[7];              //!< Array of weekdays for timer execution
    struct t_action action;        //!< Action
};

bool parse_timer_ev(struct t_list *input_devices, sds config_value);
void timer_definition_data_clear(struct t_timer_definition *timer_definition);
void timer_node_definition_data_clear(struct t_list_node *node);

#endif


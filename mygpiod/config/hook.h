/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Hook configuration
 */

#ifndef MYGPIOD_CONFIG_HOOK_H
#define MYGPIOD_CONFIG_HOOK_H

#include "mygpiod/actions/actions.h"

#include <stdbool.h>


/**
 * Config data for hooks
 */
struct t_hook {
    sds name;                      //!< Hook name
    struct t_action action;        //!< Action
};

bool parse_hook(struct t_list *hooks, sds config_value);
void hook_data_clear(struct t_hook *hook);
void hook_node_data_clear(struct t_list_node *node);

#endif


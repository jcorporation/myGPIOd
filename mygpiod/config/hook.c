/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Hook configuration
 */

#include "compile_time.h"
#include "mygpiod/config/hook.h"

#include "mygpiod/lib/list.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/sds_extras.h"

// Private definitions
static struct t_hook *new_hook(void);

// Public functions

/**
 * Parses a hook config line
 * @param hooks Pointer to list of timer hooks
 * @param config_value value to parse
 * @return true on success, else false
 */
bool parse_hook(struct t_list *hooks, sds config_value) {
    // Format is: name:action:options
    sds name = sds_getvalue(config_value, ':');
    sds action_str = sds_getvalue(config_value, ':');

    enum mygpiod_actions action = parse_action(action_str);

    // Free all parsed strings
    FREE_SDS(action_str);

    // Check if all strings could be parsed
    if (action == MYGPIOD_ACTION_UNKNOWN) {
        FREE_SDS(name);
        return false;
    }

    // Add it to the timer event list
    struct t_hook *hook = new_hook();
    hook->name = name;
    hook->action.action = action;
    if (action != MYGPIOD_ACTION_NONE) {
        hook->action.options = sdssplitargs(config_value, &hook->action.options_count);
    }
    else {
        hook->action.options = NULL;
        hook->action.options_count = 0;
    }
    return list_push(hooks, 0, hook);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param hook Timer event configuration to clear
 */
void hook_data_clear(struct t_hook *hook) {
    sdsfreesplitres(hook->action.options, hook->action.options_count);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node input config node to clear
 */
void hook_node_data_clear(struct t_list_node *node) {
    struct t_hook *hook = (struct t_hook *)node->data;
    hook_data_clear(hook);
}

// Private functions

/**
 * Mallocs and initializes a new input device struct
 * @return struct t_input_device*
 */
static struct t_hook *new_hook(void) {
    struct t_hook *hook = malloc_assert(sizeof(struct t_hook));
    return hook;
}

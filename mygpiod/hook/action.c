/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Hook action handling
 */

#include "compile_time.h"
#include "mygpiod/hook/action.h"

#include "mygpiod/actions/execute.h"
#include "mygpiod/config/hook.h"
#include "mygpiod/lib/log.h"

#include <string.h>

/**
 * Handles the configured actions for an timer event.
 * @param config Pointer to config
 * @param hook_name Hook name
 */
bool hook_action_handler(struct t_config *config, const char *hook_name) {
    MYGPIOD_LOG_DEBUG("Hook \"%s\" received", hook_name);
    struct t_list_node *current = config->hooks.head;
    while (current != NULL) {
        struct t_hook *hook = (struct t_hook *) current->data;
        if (strcmp(hook->name, hook_name) == 0) {
            MYGPIOD_LOG_INFO("Hook \"%s\" triggered.", hook_name);
            action_execute(config, &hook->action);
            return true;
        }
        current = current->next;
    }
    return false;
}

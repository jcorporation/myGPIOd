/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/execute.h"

#include "dist/sds/sds.h"
#include "mygpiod/actions/gpio.h"
#include "mygpiod/actions/system.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/sds_extras.h"

#ifdef MYGPIOD_ENABLE_ACTION_MPC
    #include "mygpiod/actions/mpc.h"
#endif

#ifdef MYGPIOD_ENABLE_ACTION_HTTP
    #include "mygpiod/actions/http.h"
    #include "mygpiod/actions/mympd.h"
#endif

#ifdef MYGPIOD_ENABLE_ACTION_LUA
    #include "mygpiod/actions/lua.h"
#endif

/**
 * Executes all configured actions for an event
 * @param config Pointer to config
 * @param actions List of actions to execute
 */
void actions_execute(struct t_config *config, struct t_list *actions) {
    struct t_list_node *current = actions->head;
    while (current != NULL) {
        struct t_action *action = (struct t_action *)current->data;
        action_execute(config, action);
        current = current->next;
    }
}

/**
 * Executes a single action
 * @param config Pointer to config
 * @param action Action to execute
 */
void action_execute(struct t_config *config, struct t_action *action) {
    sds cmd = sdsjoinsds(action->options, action->options_count, " ", 1);
    MYGPIOD_LOG_INFO("Executing %s: \"%s\"", lookup_action(action->action), cmd);
    FREE_SDS(cmd);
    switch(action->action) {
        case MYGPIOD_ACTION_SYSTEM:
            if (action->options_count != 1) {
                MYGPIOD_LOG_ERROR("Invalid number of arguments: %d", action->options_count);
                return;
            }
            action_system(action->options[0]);
            break;
        case MYGPIOD_ACTION_GPIO_SET:
            action_gpioset(config, action);
            break;
        case MYGPIOD_ACTION_GPIO_TOGGLE:
            action_gpiotoggle(config, action);
            break;
        case MYGPIOD_ACTION_GPIO_BLINK:
            action_gpioblink(config, action);
            break;
    #ifdef MYGPIOD_ENABLE_ACTION_MPC
        case MYGPIOD_ACTION_MPC:
            action_mpc(config, action);
            break;
    #endif
    #ifdef MYGPIOD_ENABLE_ACTION_HTTP
        case MYGPIOD_ACTION_HTTP:
            action_http(action);
            break;
        case MYGPIOD_ACTION_MYMPD:
            action_mympd(action);
            break;
    #endif
    #ifdef MYGPIOD_ENABLE_ACTION_LUA
        case MYGPIOD_ACTION_LUA:
            action_lua(config, action);
            break;
    #endif
        case MYGPIOD_ACTION_UNKNOWN:
            MYGPIOD_LOG_ERROR("Invalid action");
            break;
    }
}

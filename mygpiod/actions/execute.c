/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/execute.h"

#include "mygpiod/actions/gpio.h"
#include "mygpiod/actions/system.h"
#include "mygpiod/lib/action.h"
#include "mygpiod/lib/log.h"

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
 * Executes the action
 * @param actions list of actions to execute
 * @param option option for the action
 */
void action_execute(struct t_config *config, struct t_list *actions) {
    struct t_list_node *current = actions->head;
    while (current != NULL) {
        struct t_action *action = (struct t_action *)current->data;
        MYGPIOD_LOG_INFO("Executing %s: \"%s\"", lookup_action(action->action), action->option);
        switch(action->action) {
            case MYGPIOD_ACTION_SYSTEM:
                action_system(action->option);
                break;
            case MYGPIOD_ACTION_GPIO_SET:
                action_gpioset(config, action->option);
                break;
            case MYGPIOD_ACTION_GPIO_TOGGLE:
                action_gpiotoggle(config, action->option);
                break;
            case MYGPIOD_ACTION_GPIO_BLINK:
                action_gpioblink(config, action->option);
                break;
        #ifdef MYGPIOD_ENABLE_ACTION_MPC
            case MYGPIOD_ACTION_MPC:
                action_mpc(config, action->option);
                break;
        #endif
        #ifdef MYGPIOD_ENABLE_ACTION_HTTP
            case MYGPIOD_ACTION_HTTP:
                action_http(action->option);
                break;
            case MYGPIOD_ACTION_MYMPD:
                action_mympd(action->option);
                break;
        #endif
        #ifdef MYGPIOD_ENABLE_ACTION_LUA
            case MYGPIOD_ACTION_LUA:
                action_lua(config, action->option);
                break;
        #endif
            case MYGPIOD_ACTION_UNKNOWN:
                MYGPIOD_LOG_ERROR("Invalid action");
                break;
        }
        current = current->next;
    }
}

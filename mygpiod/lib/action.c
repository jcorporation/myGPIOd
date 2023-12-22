/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/lib/action.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/util.h"

#include <errno.h>
#include <strings.h>

/**
 * Clears the action node data
 * @param node pointer to client
 */
void node_data_action_clear(struct t_list_node *node) {
    struct t_action *data = (struct t_action *)node->data;
    FREE_SDS(data->option);
}

/**
 * Creates a new action node data struct
 * @param action action
 * @param option option string for the action
 * @return newly allocated action data
 */
struct t_action *action_node_data_new(enum mygpiod_actions action, sds option) {
    struct t_action *data = malloc_assert(sizeof(struct t_action));
    data->action = action;
    data->option = sdsdup(option);
    return data;
}

/**
 * Lookups the string for an action enum
 * @param action action enum
 * @return action string
 */
const char *lookup_action(enum mygpiod_actions action) {
    switch(action) {
        case MYGPIOD_ACTION_SYSTEM:
            return "system";
        case MYGPIOD_ACTION_GPIO_SET:
            return "gpioset";
        case MYGPIOD_ACTION_GPIO_TOGGLE:
            return "gpiotoggle";
        case MYGPIOD_ACTION_GPIO_BLINK:
            return "gpioblink";
        #ifdef MYGPIOD_ENABLE_ACTION_MPC
            case MYGPIOD_ACTION_MPC:
                return "mpc";
        #endif
        #ifdef MYGPIOD_ENABLE_ACTION_HTTP
            case MYGPIOD_ACTION_HTTP:
                return "http";
            case MYGPIOD_ACTION_MYMPD:
                return "mympd";
        #endif
        case MYGPIOD_ACTION_UNKNOWN:
            return "";
    }
    MYGPIOD_LOG_WARN("Could not lookup action, setting empty");
    return "";
}

/**
 * Parses a string to an action enum
 * Sets errno to EINVAL on parser error.
 * @param str string to parse
 * @return parsed action enum
 */
enum mygpiod_actions parse_action(const char *str) {
    if (strcasecmp(str, "system") == 0) {
        return MYGPIOD_ACTION_SYSTEM;
    }
    if (strcasecmp(str, "gpioset") == 0) {
        return MYGPIOD_ACTION_GPIO_SET;
    }
    if (strcasecmp(str, "gpiotoggle") == 0) {
        return MYGPIOD_ACTION_GPIO_TOGGLE;
    }
    if (strcasecmp(str, "gpioblink") == 0) {
        return MYGPIOD_ACTION_GPIO_BLINK;
    }
    #ifdef MYGPIOD_ENABLE_ACTION_MPC
        if (strcasecmp(str, "mpc") == 0) {
            return MYGPIOD_ACTION_MPC;
        }
    #endif
    #ifdef MYGPIOD_ENABLE_ACTION_HTTP
        if (strcasecmp(str, "http") == 0) {
            return MYGPIOD_ACTION_HTTP;
        }
        if (strcasecmp(str, "mympd") == 0) {
            return MYGPIOD_ACTION_MYMPD;
        }
    #endif
    errno = EINVAL;
    MYGPIOD_LOG_WARN("Could not parse action value \"%s\", setting unknown", str);
    return MYGPIOD_ACTION_UNKNOWN;
}

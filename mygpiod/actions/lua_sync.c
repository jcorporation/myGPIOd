/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Lua actions
 */

#include "compile_time.h"
#include "mygpiod/actions/lua_sync.h"

#include "mygpiod/lib/log.h"
#include "mygpiod/lua/util.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/**
 * Calls a lua function
 * @param config Pointer to config
 * @param action Action struct
 * @returns true on success, else false
 */
bool action_lua_sync(struct t_config *config, struct t_action *action) {
    if (config->lua_vm == NULL) {
        MYGPIOD_LOG_ERROR("Lua not initialized");
        return false;
    }
    if (action->options_count == 0) {
        return false;
    }
    // Push the function on the top of the lua stack
    int rv = lua_getglobal(config->lua_vm, action->options[0]);
    if (rv != 6) {
        clean_up_lua_stack(config->lua_vm);
        MYGPIOD_LOG_ERROR("\"%s\" is not a valid Lua function", action->options[0]);
        return false;
    }
    // Push the arguments on the top of the lua stack
    for (int i = 1; i < action->options_count; i++) {
        lua_pushstring(config->lua_vm, action->options[i]);
    }
    // Call the function with arguments, returning no result
    lua_call(config->lua_vm, action->options_count -1, 0);
    if (lua_gettop(config->lua_vm) == 1) {
        //return value on stack
        MYGPIOD_LOG_ERROR("%s", lua_tostring(config->lua_vm, 1));
        lua_pop(config->lua_vm, 1);
    }
    return true;
}

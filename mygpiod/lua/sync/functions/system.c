/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua system functions
 */

#include "compile_time.h"
#include "mygpiod/lua/sync/functions/system.h"

#include "mygpiod/actions/system.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lua/util.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/**
 * Executes an executable or script in a new child process.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_system_async(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "system", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *cmd = lua_tostring(lua_vm, 1);
    if (cmd == NULL) {
        MYGPIOD_LOG_ERROR("No command provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    bool rc = action_system_async(cmd);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

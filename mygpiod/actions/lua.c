/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/lua.h"

#include "mygpiod/lib/log.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/**
 * Calls a lua function
 * @param cmd command to parse
 * @returns true on success, else false
 */
bool action_lua(struct t_config *config, const char *cmd) {
    if (config->lua_vm == NULL) {
        MYGPIOD_LOG_ERROR("Lua not initialized");
        return false;
    }
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);
    if (count == 0) {
        sdsfreesplitres(args, count);
        return false;
    }
    // Push the function on the top of the lua stack
    lua_getglobal(config->lua_vm, args[0]);
    // Push the arguments on the top of the lua stack
    for (int i = 1; i < count; i++) {
        lua_pushstring(config->lua_vm, args[i]);
    }
    // Call the function with arguments, returning no result
    lua_call(config->lua_vm, count -1, 0);
    if (lua_gettop(config->lua_vm) == 1) {
        //return value on stack
        MYGPIOD_LOG_ERROR("%s", lua_tostring(config->lua_vm, 1));
        lua_pop(config->lua_vm, 1);
    }
    // Cleanup
    sdsfreesplitres(args, count);
    return true;
}

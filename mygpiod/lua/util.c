/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/lua/util.h"

#include "mygpiod/lib/log.h"

/**
 * Checks the count of required arguments and cleans up the lua stack on error
 * @param lua_vm pointer to lua vm
 * @param cmd lua command
 * @param required required argument count
 * @return true on success, else false
 */
bool check_lua_arg_count(lua_State *lua_vm, const char *cmd, int required) {
    int count = lua_gettop(lua_vm);
    if (count != required) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments (%d): \"%s\"", count, cmd);
        clean_up_lua_stack(lua_vm);
        return false;
    }
    return true;
}

/**
 * Cleans up the lua stack
 * @param lua_vm pointer to lua vm
 */
void clean_up_lua_stack(lua_State *lua_vm) {
    int count = lua_gettop(lua_vm);
    lua_pop(lua_vm, count);
}

/**
 * Gets the config struct from lua userdata
 * @param lua_vm lua instance
 * @return pointer to mympd config struct
 */
struct t_config *get_lua_global_config(lua_State *lua_vm) {
    lua_getglobal(lua_vm, "mygpiodConfig");
    struct t_config *config = (struct t_config *)lua_touserdata(lua_vm, -1);
    lua_pop(lua_vm, 1);
    return config;
}

/**
 * Set the lua rc object
 * @param lua_vm pointer to lua vm
 * @param rc Return code to set
 * @return Number of elements pushed on the Lua stack
 */
int set_lua_rc(lua_State *lua_vm, bool rc) {
    lua_pushboolean(lua_vm, rc);
    return 1;
}

/**
 * Returns a phrase for lua errors.
 * @param rc return code of the lua script
 * @return error string literal
 */
const char *lua_err_to_str(int rc) {
    switch(rc) {
        case LUA_ERRSYNTAX:
            return "Syntax error during precompilation";
        case LUA_ERRMEM:
            return "Memory allocation error";
        #if LUA_VERSION_NUM >= 503 && LUA_VERSION_NUM < 504
        case LUA_ERRGCMM:
            return "Error in garbage collector";
        #endif
        case LUA_ERRFILE:
            return "Can not open or read script file";
        case LUA_ERRRUN:
            return "Runtime error";
        case LUA_ERRERR:
            return "Error while running the message handler";
            break;
        default:
            return "Unknown error";
    }
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Lua helper functions
 */

#include "compile_time.h"
#include "mygpiod/lua/async/bytecode.h"

#include "mygpiod/lib/log.h"
#include "mygpiod/lib/sds_extras.h"
#include "mygpiod/lua/async/functions/http.h"
#include "mygpiod/lua/util.h"

// Private definitions
static int save_bytecode(lua_State *lua_vm, struct t_lua_script *script);
static lua_State *create_lua_vm(void);

// Public functions

/**
 * Loads the script from a string
 * @param script Pointer to t_lua_script
 * @return lua_State* or NULL on error
 */
lua_State *lua_async_load_source(struct t_lua_script *script) {
    lua_State *lua_vm = create_lua_vm();
    if (lua_vm == NULL) {
        return NULL;
    }
    int rc = luaL_loadstring(lua_vm, script->script);
    lua_log_result(lua_vm, rc, script->name);
    if (rc == 0) {
        save_bytecode(lua_vm, script);
        return lua_vm;
    }
    lua_close(lua_vm);
    return NULL;
}

/**
 * Loads the cached bytecode
 * This should be faster than compiling the script on each execution.
 * @param script Pointer to t_lua_script
 * @return lua_State* or NULL on error
 */
lua_State *lua_async_load_bytecode(struct t_lua_script *script) {
    lua_State *lua_vm = create_lua_vm();
    if (lua_vm == NULL) {
        return NULL;
    }
    int rc = luaL_loadbuffer(lua_vm, script->bytecode, sdslen(script->bytecode), script->name);
    if (rc == 0) {
        return lua_vm;
    }
    lua_log_result(lua_vm, rc, script->name);
    lua_close(lua_vm);
    return NULL;
}

// Private functions

/**
 * Creates the lua instance and opens the standard libraries and registers custom functions.
 * @param script_arg 
 * @return lua_State* or NULL on error
 */
static lua_State *create_lua_vm(void) {
    lua_State *lua_vm = luaL_newstate();
    if (lua_vm == NULL) {
        MYGPIOD_LOG_ERROR("Memory allocation error in luaL_newstate");
        return false;
    }
    luaL_openlibs(lua_vm);
    // Register functions
    #ifdef MYGPIOD_ENABLE_ACTION_HTTP
        lua_register(lua_vm, "http", lua_http_sync);
    #endif
    return lua_vm;
}

/**
 * Callback function for lua_dump to save the lua script bytecode
 * @param lua_vm lua state
 * @param p chunk to write
 * @param sz chunk size
 * @param ud user data
 * @return 0 on success
 */
static int dump_cb(lua_State *lua_vm, const void* p, size_t sz, void* ud) {
    (void)lua_vm;
    struct t_lua_script *script = (struct t_lua_script *)ud;
    script->bytecode = sdscatlen(script->bytecode, p, sz);
    return 0;
}

/**
 * Saves the lua bytecode
 * @param lua_vm Lua state
 * @param data Pointer to t_lua_script
 * @return 0 on success
 */
static int save_bytecode(lua_State *lua_vm, struct t_lua_script *script) {
    FREE_SDS(script->bytecode);
    script->bytecode = sdsempty();
    return lua_dump(lua_vm, dump_cb, script, false);
}

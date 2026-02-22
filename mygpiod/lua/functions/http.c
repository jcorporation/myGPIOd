/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua HTTP functions
 */

#include "compile_time.h"
#include "mygpiod/lua/functions/http.h"

#include "mygpiod/actions/http.h"
#include "mygpiod/actions/mympd.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lua/util.h"

#include <gpiod.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/**
 * Calls the myGPIOd api to execute a script in a new child process.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_mympd(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "mympd", 3) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *uri = lua_tostring(lua_vm, 1);
    if (uri == NULL) {
        MYGPIOD_LOG_ERROR("Invalid URI");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    const char *partition = lua_tostring(lua_vm, 2);
    if (partition == NULL) {
        MYGPIOD_LOG_ERROR("Invalid partition");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    const char *script = lua_tostring(lua_vm, 3);
    if (script == NULL) {
        MYGPIOD_LOG_ERROR("Invalid script");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    bool rc = action_mympd2(uri, partition, script);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

/**
 * Submits a HTTP request in a new thread.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_http(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "http", 4) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *method = lua_tostring(lua_vm, 1);
    if (method == NULL) {
        MYGPIOD_LOG_ERROR("Invalid method");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    const char *uri = lua_tostring(lua_vm, 2);
    if (uri == NULL) {
        MYGPIOD_LOG_ERROR("Invalid uri");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    const char *content_type = lua_tostring(lua_vm, 3);
    const char *postdata = lua_tostring(lua_vm, 4);
    bool rc = action_http2(method, uri, content_type, postdata);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

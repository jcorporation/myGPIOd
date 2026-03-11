/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua HTTP functions
 */

#include "compile_time.h"
#include "mygpiod/lua/async/functions/http.h"

#include "lib/http_client.h"
#include "mygpiod/lib/http_client.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lua/util.h"

#include <gpiod.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/**
 * Submits a HTTP request and returns the result.
 * This functions blocks.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_http_sync(lua_State *lua_vm) {
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
    sds resp_header = sdsempty();
    sds resp_body = sdsempty();
    bool rc = http_client(method, uri, content_type, postdata,
        &resp_header, &resp_body);
    clean_up_lua_stack(lua_vm);
    lua_pushboolean(lua_vm, rc);
    lua_pushstring(lua_vm, resp_header);
    lua_pushstring(lua_vm, resp_body);
    sdsfree(resp_header);
    sdsfree(resp_body);
    return 3;
}

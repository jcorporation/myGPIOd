/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua system functions
 */

#include "compile_time.h"
#include "mygpiod/lua/async/functions/system.h"

#include "mygpiod/lib/log.h"
#include "mygpiod/lua/util.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <unistd.h>

/**
 * Executes an executable or script and returns the result.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_system_sync(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "system", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *cmd = lua_tostring(lua_vm, 1);
    if (cmd == NULL) {
        MYGPIOD_LOG_ERROR("No command provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    sds output = sdsempty();
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    bool rc = true;
    const size_t buffer_size = 10240;
    output = sdsMakeRoomFor(output, buffer_size);
    size_t nread;
    while ((nread = fread(output + sdslen(output), sizeof(char), buffer_size, fp)) > 0) {
        sdsIncrLen(output, (ssize_t)nread);
        output = sdsMakeRoomFor(output, buffer_size);
    }
    if (ferror(fp)) {
        MYGPIOD_LOG_ERROR("Error reading from command \"%s\"", cmd);
        rc = false;
    }
    pclose(fp);
    clean_up_lua_stack(lua_vm);
    lua_pushboolean(lua_vm, rc);
    lua_pushstring(lua_vm, output);
    sdsfree(output);
    return 2;
}

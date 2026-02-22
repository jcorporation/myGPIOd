/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua MPD client functions
 */

#include "compile_time.h"
#include "mygpiod/lua/functions/mpc.h"

#include "dist/sds/sds.h"
#include "mygpiod/actions/actions.h"
#include "mygpiod/actions/mpc.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lua/util.h"

#include <gpiod.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/**
 * Runs a mpd protocol command
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_mpc(lua_State *lua_vm) {
    struct t_config *config = get_lua_global_config(lua_vm);
    if (check_lua_arg_count(lua_vm, "mpc", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *cmd = lua_tostring(lua_vm, 1);
    if (cmd == NULL) {
        MYGPIOD_LOG_ERROR("No command provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    struct t_action action;
    action.action = MYGPIOD_ACTION_MPC;
    action.options = sdssplitargs(cmd, &action.options_count);
    bool rc = action_mpc(config, &action);
    clean_up_lua_stack(lua_vm);
    sdsfreesplitres(action.options, action.options_count);
    return set_lua_rc(lua_vm, rc);
}

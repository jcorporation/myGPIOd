/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief CCustom Lua input event functions
 */

#include "compile_time.h"
#include "mygpiod/lua/async/functions/input_ev.h"

#include "mygpiod/lua/async/queue_msg.h"
#include "mygpiod/lua/sync/functions/input_ev.h"

/**
 * Async wrapper for lua_input_ev_get
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_input_ev_get_async(lua_State *lua_vm) {
    return lua_async_send_msg(lua_vm, lua_input_ev_get);
}

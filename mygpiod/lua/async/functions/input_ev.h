/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua input event functions
 */

#ifndef MYGPIOD_LUA_ASYNC_FUNCTIONS_INPUT_EV_H
#define MYGPIOD_LUA_ASYNC_FUNCTIONS_INPUT_EV_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int lua_input_ev_get_async(lua_State *lua_vm);

#endif

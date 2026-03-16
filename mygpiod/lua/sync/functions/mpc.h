/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua MPD client functions
 */

#ifndef MYGPIOD_LUA_SYNC_FUNCTIONS_MPC_H
#define MYGPIOD_LUA_SYNC_FUNCTIONS_MPC_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int lua_mpc(lua_State *lua_vm);

#endif

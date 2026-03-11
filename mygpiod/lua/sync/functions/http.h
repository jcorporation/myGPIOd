/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua HTTP functions
 */

#ifndef MYGPIOD_LUA_FUNCTIONS_HTTP_H
#define MYGPIOD_LUA_FUNCTIONS_HTTP_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>


int lua_mympd_async(lua_State *lua_vm);
int lua_http_async(lua_State *lua_vm);

#endif

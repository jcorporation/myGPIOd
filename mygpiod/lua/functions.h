/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_LUA_FUNCTIONS_H
#define MYGPIOD_LUA_FUNCTIONS_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int lua_gpio_blink(lua_State *lua_vm);
int lua_gpio_get(lua_State *lua_vm);
int lua_gpio_set(lua_State *lua_vm);
int lua_gpio_toggle(lua_State *lua_vm);
#ifdef MYGPIOD_ENABLE_ACTION_MPC
    int lua_mpc(lua_State *lua_vm);
#endif
#ifdef MYGPIOD_ENABLE_ACTION_HTTP
    int lua_mympd(lua_State *lua_vm);
    int lua_http(lua_State *lua_vm);
#endif
int lua_system(lua_State *lua_vm);

#endif

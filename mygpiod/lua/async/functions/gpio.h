/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua GPIO functions
 */

#ifndef MYGPIOD_LUA_ASYNC_FUNCTIONS_GPIO_H
#define MYGPIOD_LUA_ASYNC_FUNCTIONS_GPIO_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int lua_gpio_blink_async(lua_State *lua_vm);
int lua_gpio_get_async(lua_State *lua_vm);
int lua_gpio_set_async(lua_State *lua_vm);
int lua_gpio_toggle_async(lua_State *lua_vm);

#endif

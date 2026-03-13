/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Async Lua VM and bytecode functions
 */

#ifndef MYGPIOD_LUA_ASYNC_BYTECODE_H
#define MYGPIOD_LUA_ASYNC_BYTECODE_H

#include "mygpiod/config/lua_async.h"

#include "mygpiod/config/config.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>

lua_State *lua_async_load_source(struct t_config *config, struct t_lua_script *script);
lua_State *lua_async_load_bytecode(struct t_config *config, struct t_lua_script *script);

#endif

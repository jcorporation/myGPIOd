/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Lua helper functions
 */

#ifndef MYGPIOD_LUA_UTIL_H
#define MYGPIOD_LUA_UTIL_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>

struct t_config *get_lua_global_config(lua_State *lua_vm);
int set_lua_rc(lua_State *lua_vm, bool rc);
void clean_up_lua_stack(lua_State *lua_vm);
bool check_lua_arg_count(lua_State *lua_vm, const char *cmd, int required);
const char *lua_err_to_str(int rc);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Message handling from async lua thread
 */

#ifndef MYGPIOD_LUA_ASYNC_HANDLE_MSG_H
#define MYGPIOD_LUA_ASYNC_HANDLE_MSG_H

#include <lua.h>
#include <stdbool.h>

/**
 * Typedef for calling a C function called by Lua
 */
typedef int (*t_lua_func) (lua_State *);

/**
 * Request struct from Lua thread to main thread
 */
struct t_lua_async_request {
    lua_State *lua_vm;    //!< Lua VM from thread
    t_lua_func lua_func;  //!< C function to call from Lua
};

/**
 * Response struct from main thread to Lua thread
 */
struct t_lua_async_response {
    int rc;  //!< Number of values on the stack
};

bool lua_async_handle_msg(int *fd);
int lua_async_send_msg(lua_State *lua_vm, t_lua_func lua_func);

struct t_lua_async_request *lua_async_request_new(lua_State *lua_vm, t_lua_func lua_func);
void lua_async_request_free(void *data);
struct t_lua_async_response *lua_async_response_new(int rc);
void lua_async_response_free(void *data);

#endif

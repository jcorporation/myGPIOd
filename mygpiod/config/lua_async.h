/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Lua async configuration
 */

#ifndef MYGPIOD_CONFIG_LUA_ASYNC_H
#define MYGPIOD_CONFIG_LUA_ASYNC_H


#include "dist/sds/sds.h"
#include "mygpiod/lib/list.h"

#include <stdbool.h>

/**
 * Config data for hooks
 */
struct t_lua_script {
    sds name;      //!< Script name
    sds script;    //!< Script source
    sds bytecode;  //!< Compiled script
};

bool lua_async_read_scripts(struct t_list *lua_async_scripts, sds config_value);
void lua_async_data_clear(struct t_lua_script *data);
void lua_async_node_data_clear(struct t_list_node *node);

#endif

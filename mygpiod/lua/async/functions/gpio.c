/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua functions
 */

#include "compile_time.h"
#include "mygpiod/lua/async/functions/gpio.h"

#include "mygpiod/lua/async/queue_msg.h"
#include "mygpiod/lua/sync/functions/gpio.h"

/**
 * Async wrapper for lua_gpio_blink
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_gpio_blink_async(lua_State *lua_vm) {
    return lua_async_send_msg(lua_vm, lua_gpio_blink);
}

/**
 * Async wrapper for lua_gpio_get
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_gpio_get_async(lua_State *lua_vm) {
    return lua_async_send_msg(lua_vm, lua_gpio_get);
}

/**
 * Async wrapper for lua_gpio_set
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_gpio_set_async(lua_State *lua_vm) {
    return lua_async_send_msg(lua_vm, lua_gpio_set);
}

/**
 * Async wrapper for lua_gpio_toggle
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_gpio_toggle_async(lua_State *lua_vm) {
    return lua_async_send_msg(lua_vm, lua_gpio_toggle);
}

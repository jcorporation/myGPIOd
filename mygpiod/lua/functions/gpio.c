/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua functions
 */

#include "compile_time.h"
#include "mygpiod/lua/functions/gpio.h"

#include "mygpiod/gpio/gpio.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/gpio/util.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lua/util.h"

#include <gpiod.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/**
 * Toggle the value of the GPIO in given timeout and interval.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_gpio_blink(lua_State *lua_vm) {
    struct t_config *config = get_lua_global_config(lua_vm);
    if (check_lua_arg_count(lua_vm, "gpioBlink", 3) == false) {
        return set_lua_rc(lua_vm, false);
    }
    if (lua_isinteger(lua_vm, 1) == 0) {
        MYGPIOD_LOG_ERROR("Argument GPIO is not a number");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    if (lua_isinteger(lua_vm, 2) == 0) {
        MYGPIOD_LOG_ERROR("Argument timeout is not a number");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    if (lua_isinteger(lua_vm, 3) == 0) {
        MYGPIOD_LOG_ERROR("Argument interval is not a number");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    unsigned gpio = (unsigned)lua_tointeger(lua_vm, 1);
    int timeout = (int)lua_tointeger(lua_vm, 2);
    int interval = (int)lua_tointeger(lua_vm, 3);
    bool rc = gpio_blink(config, gpio, timeout, interval);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

/**
 * Gets a GPIO value.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_gpio_get(lua_State *lua_vm) {
    struct t_config *config = get_lua_global_config(lua_vm);
    if (check_lua_arg_count(lua_vm, "gpioGet", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    if (lua_isinteger(lua_vm, 1) == 0) {
        MYGPIOD_LOG_ERROR("Argument GPIO is not a number");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    unsigned gpio = (unsigned)lua_tointeger(lua_vm, 1);
    enum gpiod_line_value value = gpio_get_value(config, gpio);
    clean_up_lua_stack(lua_vm);
    lua_pushboolean(lua_vm, true);
    lua_pushstring(lua_vm, lookup_gpio_value(value));
    return 2;
}

/**
 * Sets a GPIO value.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_gpio_set(lua_State *lua_vm) {
    struct t_config *config = get_lua_global_config(lua_vm);
    if (check_lua_arg_count(lua_vm, "gpioSet", 2) == false) {
        return set_lua_rc(lua_vm, false);
    }
    if (lua_isinteger(lua_vm, 1) == 0) {
        MYGPIOD_LOG_ERROR("Argument GPIO is not a number");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    unsigned gpio = (unsigned)lua_tointeger(lua_vm, 1);
    const char *value_str = lua_tostring(lua_vm, 2);
    if (value_str == NULL) {
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    enum gpiod_line_value value = parse_gpio_value(value_str);
    if (value == GPIOD_LINE_VALUE_ERROR) {
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    bool rc = gpio_set_value(config, gpio, value);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

/**
 * Toggles a GPIO value.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_gpio_toggle(lua_State *lua_vm) {
    struct t_config *config = get_lua_global_config(lua_vm);
    if (check_lua_arg_count(lua_vm, "gpioToggle", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    if (lua_isinteger(lua_vm, 1) == 0) {
        MYGPIOD_LOG_ERROR("Argument GPIO is not a number");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    unsigned gpio = (unsigned)lua_tointeger(lua_vm, 1);
    bool rc = gpio_toggle_value(config, gpio);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

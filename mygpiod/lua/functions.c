/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/lua/functions.h"

#include "dist/sds/sds.h"
#include "lib/action.h"
#include "mygpiod/actions/http.h"
#include "mygpiod/actions/mpc.h"
#include "mygpiod/actions/mympd.h"
#include "mygpiod/actions/system.h"
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

#ifdef MYGPIOD_ENABLE_ACTION_MPC
/**
 * Runs a mpd protocol command
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_mpc(lua_State *lua_vm) {
    struct t_config *config = get_lua_global_config(lua_vm);
    if (check_lua_arg_count(lua_vm, "mpc", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *cmd = lua_tostring(lua_vm, 1);
    if (cmd == NULL) {
        MYGPIOD_LOG_ERROR("No command provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    struct t_action action;
    action.action = MYGPIOD_ACTION_MPC;
    action.options = sdssplitargs(cmd, &action.options_count);
    bool rc = action_mpc(config, &action);
    clean_up_lua_stack(lua_vm);
    sdsfreesplitres(action.options, action.options_count);
    return set_lua_rc(lua_vm, rc);
}
#endif

#ifdef MYGPIOD_ENABLE_ACTION_HTTP
/**
 * Calls the myGPIOd api to execute a script in a new child process.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_mympd(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "mympd", 3) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *uri = lua_tostring(lua_vm, 1);
    if (uri == NULL) {
        MYGPIOD_LOG_ERROR("Invalid URI");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    const char *partition = lua_tostring(lua_vm, 2);
    if (partition == NULL) {
        MYGPIOD_LOG_ERROR("Invalid partition");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    const char *script = lua_tostring(lua_vm, 3);
    if (script == NULL) {
        MYGPIOD_LOG_ERROR("Invalid script");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    bool rc = action_mympd2(uri, partition, script);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

/**
 * Submits a HTTP request in a new thread.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_http(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "http", 4) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *method = lua_tostring(lua_vm, 1);
    if (method == NULL) {
        MYGPIOD_LOG_ERROR("Invalid method");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    const char *uri = lua_tostring(lua_vm, 2);
    if (uri == NULL) {
        MYGPIOD_LOG_ERROR("Invalid uri");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    const char *content_type = lua_tostring(lua_vm, 3);
    const char *postdata = lua_tostring(lua_vm, 4);
    bool rc = action_http2(method, uri, content_type, postdata);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}
#endif

/**
 * Executes an executable or script in a new child process.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_system(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "system", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *cmd = lua_tostring(lua_vm, 1);
    if (cmd == NULL) {
        MYGPIOD_LOG_ERROR("No command provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    bool rc = action_system(cmd);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

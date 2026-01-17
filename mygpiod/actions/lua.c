/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/lua.h"

#include "actions/http.h"
#include "actions/mpc.h"
#include "actions/mympd.h"
#include "actions/system.h"
#include "mygpiod/gpio/gpio.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/gpio/util.h"
#include "mygpiod/lib/log.h"

#include <gpiod.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

// private definitions
static struct t_config *get_lua_global_config(lua_State *lua_vm);
static int set_lua_rc(lua_State *lua_vm, bool rc);
static void clean_up_lua_stack(lua_State *lua_vm);
static bool check_lua_arg_count(lua_State *lua_vm, const char *cmd, int required);
static int lua_gpio_blink(lua_State *lua_vm);
static int lua_gpio_get(lua_State *lua_vm);
static int lua_gpio_set(lua_State *lua_vm);
static int lua_gpio_toggle(lua_State *lua_vm);
#ifdef MYGPIOD_ENABLE_ACTION_MPC
    static int lua_mpc(lua_State *lua_vm);
#endif
#ifdef MYGPIOD_ENABLE_ACTION_HTTP
    static int lua_mympd(lua_State *lua_vm);
    static int lua_http(lua_State *lua_vm);
#endif
static int lua_system(lua_State *lua_vm);
static const char *lua_err_to_str(int rc);

// public functions

/**
 * Initializes the lua vm and loads the lua file with user defined functions
 * @param config pointer to config
 * @return true on success, else false
 */
bool init_luavm(struct t_config *config) {
    if (sdslen(config->lua_file) == 0) {
        MYGPIOD_LOG_DEBUG("No lua file configured");
        return true;
    }
    config->lua_vm = luaL_newstate();
    if (config->lua_vm == NULL) {
        MYGPIOD_LOG_ERROR("Unable to create lua state");
        return false;
    }
    // Load Lua base libraries
    luaL_openlibs(config->lua_vm);
    // Set config as a global
    lua_pushlightuserdata(config->lua_vm, config);
    lua_setglobal(config->lua_vm, "mygpiodConfig");
    // Register functions
    lua_register(config->lua_vm, "gpioBlink", lua_gpio_blink);
    lua_register(config->lua_vm, "gpioGet", lua_gpio_get);
    lua_register(config->lua_vm, "gpioSet", lua_gpio_set);
    lua_register(config->lua_vm, "gpioToggle", lua_gpio_toggle);
    lua_register(config->lua_vm, "system", lua_system);
    #ifdef MYGPIOD_ENABLE_ACTION_MPC
        lua_register(config->lua_vm, "mpc", lua_mpc);
    #endif
    #ifdef MYGPIOD_ENABLE_ACTION_HTTP
        lua_register(config->lua_vm, "mympd", lua_mympd);
        lua_register(config->lua_vm, "http", lua_http);
    #endif
    // Load user defined lua file
    int rc = luaL_dofile(config->lua_vm, config->lua_file);
    if (rc != 0) {
        const char *err_str = lua_err_to_str(rc);
        MYGPIOD_LOG_ERROR("Error loading lua file: %s", err_str);
        if (lua_gettop(config->lua_vm) == 1) {
            //return value on stack
            MYGPIOD_LOG_ERROR("%s", lua_tostring(config->lua_vm, 1));
        }
        lua_close(config->lua_vm);
        config->lua_vm = NULL;
        return false;
    }
    MYGPIOD_LOG_INFO("Lua initialized");
    return true;
}

/**
 * Calls a lua function
 * @param cmd command to parse
 * @returns true on success, else false
 */
bool action_lua(struct t_config *config, const char *cmd) {
    if (config->lua_vm == NULL) {
        MYGPIOD_LOG_ERROR("Lua not initialized");
        return false;
    }
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);
    if (count == 0) {
        sdsfreesplitres(args, count);
        return false;
    }
    // Push the function on the top of the lua stack
    lua_getglobal(config->lua_vm, args[0]);
    // Push the arguments on the top of the lua stack
    for (int i = 1; i < count; i++) {
        lua_pushstring(config->lua_vm, args[i]);
    }
    // Call the function with arguments, returning no result
    lua_call(config->lua_vm, count -1, 0);
    if (lua_gettop(config->lua_vm) == 1) {
        //return value on stack
        MYGPIOD_LOG_ERROR("%s", lua_tostring(config->lua_vm, 1));
        lua_pop(config->lua_vm, 1);
    }
    // Cleanup
    sdsfreesplitres(args, count);
    return true;
}

// private functions

/**
 * Checks the count of required arguments and cleans up the lua stack on error
 * @param lua_vm pointer to lua vm
 * @param cmd lua command
 * @param required required argument count
 * @return true on success, else false
 */
static bool check_lua_arg_count(lua_State *lua_vm, const char *cmd, int required) {
    int count = lua_gettop(lua_vm);
    if (count != required) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments (%d): \"%s\"", count, cmd);
        clean_up_lua_stack(lua_vm);
        return false;
    }
    return true;
}

/**
 * Cleans up the lua stack
 * @param lua_vm pointer to lua vm
 */
static void clean_up_lua_stack(lua_State *lua_vm) {
    int count = lua_gettop(lua_vm);
    lua_pop(lua_vm, count);
}

/**
 * Gets the config struct from lua userdata
 * @param lua_vm lua instance
 * @return pointer to mympd config struct
 */
static struct t_config *get_lua_global_config(lua_State *lua_vm) {
    lua_getglobal(lua_vm, "mygpiodConfig");
    struct t_config *config = (struct t_config *)lua_touserdata(lua_vm, -1);
    lua_pop(lua_vm, 1);
    return config;
}

/**
 * Set the lua rc object
 * @param lua_vm pointer to lua vm
 * @param rc Return code to set
 * @return Number of elements pushed on the Lua stack
 */
static int set_lua_rc(lua_State *lua_vm, bool rc) {
    lua_pushboolean(lua_vm, rc);
    return 1;
}

/**
 * Toggle the value of the GPIO in given timeout and interval.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
static int lua_gpio_blink(lua_State *lua_vm) {
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
    int rc = gpio_blink(config, gpio, timeout, interval);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

/**
 * Gets a GPIO value.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
static int lua_gpio_get(lua_State *lua_vm) {
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
static int lua_gpio_set(lua_State *lua_vm) {
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
    int rc = gpio_set_value(config, gpio, value);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

/**
 * Toggles a GPIO value.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
static int lua_gpio_toggle(lua_State *lua_vm) {
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
    int rc = gpio_toggle_value(config, gpio);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

#ifdef MYGPIOD_ENABLE_ACTION_MPC
/**
 * Runs a mpd protocol command
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
static int lua_mpc(lua_State *lua_vm) {
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
    int rc = action_mpc(config, cmd);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}
#endif

#ifdef MYGPIOD_ENABLE_ACTION_HTTP
/**
 * Calls the myGPIOd api to execute a script in a new child process.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
static int lua_mympd(lua_State *lua_vm) {
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
    int rc = action_mympd2(uri, partition, script);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

/**
 * Submits a HTTP request in a new thread.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
static int lua_http(lua_State *lua_vm) {
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
    int rc = action_http2(method, uri, content_type, postdata);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}
#endif

/**
 * Executes an executable or script in a new child process.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
static int lua_system(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "system", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *cmd = lua_tostring(lua_vm, 1);
    if (cmd == NULL) {
        MYGPIOD_LOG_ERROR("No command provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    int rc = action_system(cmd);
    clean_up_lua_stack(lua_vm);
    return set_lua_rc(lua_vm, rc);
}

/**
 * Returns a phrase for lua errors.
 * @param rc return code of the lua script
 * @return error string literal
 */
static const char *lua_err_to_str(int rc) {
    switch(rc) {
        case LUA_ERRSYNTAX:
            return "Syntax error during precompilation";
        case LUA_ERRMEM:
            return "Memory allocation error";
        #if LUA_VERSION_NUM >= 503 && LUA_VERSION_NUM < 504
        case LUA_ERRGCMM:
            return "Error in garbage collector";
        #endif
        case LUA_ERRFILE:
            return "Can not open or read script file";
        case LUA_ERRRUN:
            return "Runtime error";
        case LUA_ERRERR:
            return "Error while running the message handler";
            break;
        default:
            return "Unknown error";
    }
}

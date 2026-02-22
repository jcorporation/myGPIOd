/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Lua VM
 */

#include "compile_time.h"
#include "mygpiod/lua/luavm.h"

#include "mygpiod/lib/log.h"
#include "mygpiod/lua/functions/gpio.h"
#include "mygpiod/lua/functions/input_ev.h"
#include "mygpiod/lua/functions/system.h"
#include "mygpiod/lua/util.h"

#ifdef MYGPIOD_ENABLE_ACTION_MPC
    #include "mygpiod/lua/functions/mpc.h"
#endif
#ifdef MYGPIOD_ENABLE_ACTION_HTTP
    #include "mygpiod/lua/functions/http.h"
#endif

#include <gpiod.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/**
 * Initializes the lua vm and loads the lua file with user defined functions
 * @param config pointer to config
 * @return true on success, else false
 */
bool luavm_init(struct t_config *config) {
    if (sdslen(config->lua_file) == 0) {
        MYGPIOD_LOG_INFO("No lua file configured");
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
    lua_register(config->lua_vm, "inputEvGet", lua_input_ev_get);
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
        MYGPIOD_LOG_ERROR("Failure loading lua file: \"%s\"", err_str);
        if (lua_gettop(config->lua_vm) == 1) {
            //return value on stack
            MYGPIOD_LOG_ERROR("%s", lua_tostring(config->lua_vm, 1));
        }
        lua_close(config->lua_vm);
        config->lua_vm = NULL;
        return false;
    }
    MYGPIOD_LOG_INFO("Lua initialized successfully");
    return true;
}

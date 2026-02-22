/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief CCustom Lua input event functions
 */

#include "compile_time.h"
#include "mygpiod/lua/functions/input_ev.h"

#include "mygpiod/input_ev/action.h"
#include "mygpiod/input_ev/device.h"
#include "mygpiod/input_ev/event_code.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lua/util.h"

#include <gpiod.h>
#include <lauxlib.h>
#include <linux/input-event-codes.h>
#include <lua.h>
#include <lualib.h>

/**
 * Gets the current state of an input event
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_input_ev_get(lua_State *lua_vm) {
    struct t_config *config = get_lua_global_config(lua_vm);
    if (check_lua_arg_count(lua_vm, "input_ev_get", 2) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *device_name = lua_tostring(lua_vm, 1);
    if (device_name == NULL) {
        MYGPIOD_LOG_ERROR("No input device provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    struct t_input_device *device = input_device_get_by_name(&config->input_devices, device_name);
    if (device == NULL) {
        MYGPIOD_LOG_ERROR("Input device \"%s\" not configured", device_name);
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }

    const char *code_str = lua_tostring(lua_vm, 2);
    if (code_str == NULL) {
        MYGPIOD_LOG_ERROR("No input code provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    unsigned short code = input_event_code_parse(code_str);
    if (code == KEY_MAX) {
        MYGPIOD_LOG_ERROR("Invalid input code provided: \"%s\"", code_str);
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }

    unsigned value = input_ev_get_state(device, code);
    clean_up_lua_stack(lua_vm);
    lua_pushboolean(lua_vm, (value == UINT_MAX ? false : true));
    lua_pushinteger(lua_vm, value);
    return 2;
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Async Lua actions
 */

#include "compile_time.h"
#include "mygpiod/actions/lua_async.h"

#include "lib/mem.h"
#include "mygpiod/config/lua_async.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lua/async/bytecode.h"
#include "mygpiod/lua/util.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// Private definitions
static void *script_run(void *script_thread_arg);
static struct t_lua_script *get_script_by_name(struct t_list *lua_async_scripts, const char *name);

/**
 * Struct for thread arguments
 */
struct t_script_run_arg {
    lua_State *lua_vm;  //!< Lua VM
    sds script_name;    //!< Script name
};

// Public functions

/**
 * Calls an async Lua function
 * @param config Pointer to config
 * @param action Action struct
 * @returns true on success, else false
 */
bool action_lua_async(struct t_config *config, struct t_action *action) {
    if (action->options_count == 0) {
        return false;
    }
    struct t_lua_script *script = get_script_by_name(&config->lua_async_scripts, action->options[0]);
    if (script == NULL) {
        return false;
    }

    struct t_script_run_arg *script_thread_arg = malloc_assert(sizeof(struct t_script_run_arg));
    script_thread_arg->lua_vm = script->bytecode == NULL
        ? lua_async_load_source(script)
        : lua_async_load_bytecode(script);
    if (script_thread_arg->lua_vm == NULL) {
        free(script_thread_arg);
        return false;
    }
    script_thread_arg->script_name = script->name;

    // Execute script in new thread
    pthread_t scripts_worker_thread;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0 ||
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 ||
        pthread_create(&scripts_worker_thread, &attr, script_run, script_thread_arg) != 0)
    {
        MYGPIOD_LOG_ERROR("Failure creating Lua thread.");
        lua_close(script_thread_arg->lua_vm);
        free(script_thread_arg);
        return false;
    }
    return true;
}

// Private functions

/**
 * Main function for async Lua thread
 * @param script_thread_arg 
 * @return void* NULL
 */
static void *script_run(void *script_thread_arg) {
    logline = sdsempty();
    struct t_script_run_arg *script = (struct t_script_run_arg *) script_thread_arg;

    MYGPIOD_LOG_DEBUG("Start async Lua script \"%s\"", script->script_name);
    bool rc = lua_pcall(script->lua_vm, 0, 1, 0);
    MYGPIOD_LOG_DEBUG("End async Lua script \"%s\"", script->script_name);
    if (rc == 1) {
        lua_log_result(script->lua_vm, rc, script->script_name);
    }
    lua_close(script->lua_vm);
    free(script);

    sdsfree(logline);
    return NULL;
}

/**
 * Get the script by name object
 * @param lua_async_scripts Pointer to listof async Lua scripts
 * @param name Script name
 * @return struct t_lua_script* or NULL if not found
 */
static struct t_lua_script *get_script_by_name(struct t_list *lua_async_scripts, const char *name) {
    struct t_list_node *current = lua_async_scripts->head;
    while (current != NULL) {
        struct t_lua_script *script = (struct t_lua_script *)current->data;
        if (strcmp(script->name, name) == 0) {
            return script;
        }
        current = current->next;
    }
    return NULL;
}

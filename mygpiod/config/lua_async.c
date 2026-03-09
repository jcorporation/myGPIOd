/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Hook configuration
 */

#include "compile_time.h"
#include "mygpiod/config/lua_async.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/sds_extras.h"

#include <dirent.h>
#include <errno.h>
#include <string.h>

// Private definitions
static struct t_lua_script *new_script(void);

// Public functions

/**
 * Reads all Lua files from the lua_async_dir
 * @param hooks Pointer to list of timer hooks
 * @param config_value value to parse
 * @return true on success, else false
 */
bool lua_async_read_scripts(struct t_list *lua_async_scripts, sds lua_async_dir) {
    errno = 0;
    DIR *script_dir = opendir(lua_async_dir);
    if (script_dir == NULL) {
        MYGPIOD_LOG_ERROR("Can not open directory \"%s\"", lua_async_dir);
        MYGPIOD_LOG_ERRNO(errno);
        return false;
    }

    sds file_path = sdsempty();
    struct dirent *next_file;
    while ((next_file = readdir(script_dir)) != NULL ) {
        if (next_file->d_type != DT_REG) {
            continue;
        }
        const char *ext = strrchr(next_file->d_name, '.');
        if (ext == NULL ||
            strcmp(ext, ".lua") != 0)
        {
            continue;
        }
        struct t_lua_script *data = new_script();
        data->name = sdsnewlen(next_file->d_name, strlen(next_file->d_name) - 4);
        data->bytecode = NULL;
        int nread;
        file_path = sdscatfmt(file_path, "%s/%s", lua_async_dir, next_file->d_name);
        data->script = sds_getfile(sdsempty(), file_path, &nread);
        if (nread > 0) {
            MYGPIOD_LOG_DEBUG("Adding async Lua script: \"%s\"", data->name);
            list_push(lua_async_scripts, 0, data);
        }
        else {
            lua_async_data_clear(data);
            free(data);
        }
        sdsclear(file_path);
    }
    closedir(script_dir);
    FREE_SDS(file_path);
    MYGPIOD_LOG_INFO("Read %u async Lua script(s) from disc", lua_async_scripts->length);
    return true;
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param hook Timer event configuration to clear
 */
void lua_async_data_clear(struct t_lua_script *data) {
    sdsfree(data->bytecode);
    sdsfree(data->name);
    sdsfree(data->script);
}

/**
 * Frees pointers from this node.
 * @param node input config node to clear
 */
void lua_async_node_data_clear(struct t_list_node *node) {
    struct t_lua_script *data = (struct t_lua_script *)node->data;
    lua_async_data_clear(data);
}

// Private functions

/**
 * Mallocs and initializes a new input device struct
 * @return struct t_input_device*
 */
static struct t_lua_script *new_script(void) {
    struct t_lua_script *data = malloc_assert(sizeof(struct t_lua_script));
    return data;
}

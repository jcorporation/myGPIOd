/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Custom Lua MPD client functions
 */

#include "compile_time.h"
#include "mygpiod/lua/async/functions/mpc.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lua/util.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <mpd/client.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>

/**
 * Connects to mpd, runs a protocol command, reads it outputs
 * and disconnects.
 * @param lua_vm pointer to lua vm
 * @return Number of values on the stack
 */
int lua_mpc_async(lua_State *lua_vm) {
    if (check_lua_arg_count(lua_vm, "mpc", 1) == false) {
        return set_lua_rc(lua_vm, false);
    }
    const char *cmd = lua_tostring(lua_vm, 1);
    if (cmd == NULL) {
        MYGPIOD_LOG_ERROR("No command provided");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }

    struct mpd_connection *mpd_conn = mpd_connection_new(NULL, 0, 0);
    if (mpd_conn == NULL) {
        MYGPIOD_LOG_ERROR("MPD Connection: Out of memory");
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }
    if (mpd_connection_get_error(mpd_conn) != MPD_ERROR_SUCCESS) {
        MYGPIOD_LOG_ERROR("MPD Connection error: %s", mpd_connection_get_error_message(mpd_conn));
        mpd_connection_free(mpd_conn);
        clean_up_lua_stack(lua_vm);
        return set_lua_rc(lua_vm, false);
    }

    int count;
    sds *options = sdssplitargs(cmd, &count);
    const int max_mpc_args = 10;  //!< Maximum number of arguments for mpd_send_command
    sds *tokens = malloc_assert(sizeof(sds)*(size_t)max_mpc_args);
    for (int i = 0; i < max_mpc_args; i++) {
        tokens[i] = i < count
            ? options[i]
            : NULL;
    }
    bool rc = mpd_send_command(mpd_conn, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4],
        tokens[5], tokens[6], tokens[7], tokens[8], tokens[9], NULL);
    sdsfreesplitres(options, count);
    free(tokens);

    sds output = sdsempty();
    if (rc == true) {
        rc = false;
        struct pollfd fd;
        fd.fd = mpd_connection_get_fd(mpd_conn);
        fd.events = POLLIN | POLLPRI;
        if (poll(&fd, 1, 60000) > -1) {
            ssize_t nread;
            size_t oldlen = 0;
            const size_t buffer_size = 10240;
            output = sdsMakeRoomFor(output, buffer_size);
            while ((nread = read(fd.fd, output + oldlen, buffer_size)) > 0) {
                sdsIncrLen(output, nread);
                output = sdsMakeRoomFor(output, buffer_size);
            }
        }
        // Check MPD response
        if (sdslen(output) > 3) {
            const char *mpd_return_code = output + (sdslen(output) - 3);
            if (strncmp(mpd_return_code, "OK", 2) == 0) {
                rc = true;
            }
        }
    }

    mpd_connection_free(mpd_conn);
    clean_up_lua_stack(lua_vm);
    lua_pushboolean(lua_vm, rc);
    lua_pushstring(lua_vm, output);
    sdsfree(output);
    return 2;
}

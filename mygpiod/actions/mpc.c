/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/mpc.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"

#include <errno.h>
#include <mpd/client.h>
#include <string.h>
#include <unistd.h>

// private definitions
static void run_mpc(const char *cmd);

// public functions

/**
 * Controls MPD in a new process
 * @param cmd command to parse
 * @returns true on success, else false
 */
bool action_mpc(const char *cmd) {
    errno = 0;
    int pid = fork();
    if (pid == 0) {
        // this is the child process
        run_mpc(cmd);
        exit(EXIT_SUCCESS);
    }
    else {
        if (pid == -1) {
            MYGPIOD_LOG_ERROR("Could not fork: %s", strerror(errno));
            return false;
        }
        MYGPIOD_LOG_DEBUG("Forked process with pid %d", pid);
        return true;
    }
}

// private functions

/**
 * Connects to MPD and runs the command
 * @param cmd command and it's options, format:
 *            {mpd command} [{option1} {option2} ...]
 *            A maximum of 10 options are supported.
 * @return true on success, else false
 */
static void run_mpc(const char *cmd) {
    #define MAX_MPC_ARGS 10
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);
    if (count < 1 || count > MAX_MPC_ARGS) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        sdsfreesplitres(args, count);
        exit(EXIT_FAILURE);
    }
    sds *tokens = malloc_assert(sizeof(sds)*MAX_MPC_ARGS);
    for (int i = 0; i < MAX_MPC_ARGS; i++) {
        tokens[i] = i < count
            ? args[i]
            : NULL;
    }

    // Use default connection settings
    struct mpd_connection *conn = mpd_connection_new(NULL, 0, 0);
    if (conn == NULL) {
        MYGPIOD_LOG_ERROR("MPD Connection: Out of memory");
        sdsfreesplitres(args, count);
        free(tokens);
        exit(EXIT_FAILURE);
    }
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
        MYGPIOD_LOG_ERROR("MPD Connection error: %s", mpd_connection_get_error_message(conn));
        mpd_connection_free(conn);
        sdsfreesplitres(args, count);
        free(tokens);
        exit(EXIT_FAILURE);
    }
    // We are connected
    mpd_send_command(conn, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4],
        tokens[5], tokens[6], tokens[7], tokens[8], tokens[9], NULL);
    sdsfreesplitres(args, count);
    free(tokens);
    if (mpd_response_finish(conn) == false) {
        MYGPIOD_LOG_ERROR("MPD error: %s", mpd_connection_get_error_message(conn));
    }
    mpd_connection_free(conn);
}

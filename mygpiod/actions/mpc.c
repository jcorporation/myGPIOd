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
#include <poll.h>
#include <string.h>

// private definitions
static bool mpc_connect(struct t_config *config);
static void mpc_disconnect(struct t_config *config);
static bool mpc_check_conn(struct t_config *config);

// public functions

/**
 * Controls MPD, re-uses the existing connection or creates a new one
 * @param cmd command to parse
 * @returns true on success, else false
 */
bool action_mpc(struct t_config *config, const char *cmd) {
    #define MAX_MPC_ARGS 10
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);
    if (count < 1 || count > MAX_MPC_ARGS) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        sdsfreesplitres(args, count);
        return false;
    }
    sds *tokens = malloc_assert(sizeof(sds)*MAX_MPC_ARGS);
    for (int i = 0; i < MAX_MPC_ARGS; i++) {
        tokens[i] = i < count
            ? args[i]
            : NULL;
    }

    // Check connection
    if (mpc_check_conn(config) == false &&
        mpc_connect(config) == false)
    {
        sdsfreesplitres(args, count);
        free(tokens);
        return false;
    }
    // We are connected
    mpd_send_command(config->mpd_conn, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4],
        tokens[5], tokens[6], tokens[7], tokens[8], tokens[9], NULL);
    sdsfreesplitres(args, count);
    free(tokens);
    if (mpd_response_finish(config->mpd_conn) == false) {
        MYGPIOD_LOG_ERROR("MPD error: %s", mpd_connection_get_error_message(config->mpd_conn));
        mpc_disconnect(config);
    }
    return true;
}

// private functions

/**
 * Checks if the MPD connection is ready
 * @param config pointer to config struct
 * @return true if it ready, else false 
 */
static bool mpc_check_conn(struct t_config *config) {
    if (config->mpd_conn == NULL) {
        MYGPIOD_LOG_DEBUG("MPD is disconnected");
        return false;
    }
    struct pollfd fd;
    fd.fd = mpd_connection_get_fd(config->mpd_conn);
    fd.events = 0;
    if (poll(&fd, 1, 0) > -1 &&
        !(fd.revents && POLLHUP | POLLERR))
    {
        MYGPIOD_LOG_DEBUG("MPD is connected");
        return true;
    }
    MYGPIOD_LOG_DEBUG("MPD is disconnected");
    mpc_disconnect(config);
    return false;
}

/**
 * Connects to MPD with default libmpdclient settings
 * @param config pointer to config struct
 * @return true on success, else false
 */
static bool mpc_connect(struct t_config *config) {
    config->mpd_conn = mpd_connection_new(NULL, 0, 0);
    if (config->mpd_conn == NULL) {
        MYGPIOD_LOG_ERROR("MPD Connection: Out of memory");       
        return false;
    }
    if (mpd_connection_get_error(config->mpd_conn) != MPD_ERROR_SUCCESS) {
        MYGPIOD_LOG_ERROR("MPD Connection error: %s", mpd_connection_get_error_message(config->mpd_conn));
        mpc_disconnect(config);
        return false;
    }
    MYGPIOD_LOG_DEBUG("New MPD connection established");
    return true;
}

/**
 * Disconnects from MPD
 * @param config pointer to config struct
 */
static void mpc_disconnect(struct t_config *config) {
    if (config->mpd_conn != NULL) {
        MYGPIOD_LOG_DEBUG("Freeing MPD connection");
        mpd_connection_free(config->mpd_conn);
        config->mpd_conn = NULL;
    }
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief myMPD actions
 */

#include "compile_time.h"
#include "mygpiod/actions/mympd.h"

#include "dist/sds/sds.h"
#include "mygpiod/actions/http.h"
#include "mygpiod/lib/log.h"

/**
 * Execute a myGPIOd script through the myGPIOd api.
 * @param action Action struct, options must be:
 *            {uri} {partition} {script}
 * @returns true on success, else false
 */
bool action_mympd(struct t_action *action) {
    if (action->options_count < 3) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments: %d", action->options_count);
        return false;
    }
    return action_mympd2(action->options[0], action->options[1], action->options[2]);
}

/**
 * Execute a myGPIOd script through the myGPIOd api.
 * @param uri myGPIOd uri
 * @param partition MPD partition
 * @param script script name
 * @returns true on success, else false
 */
bool action_mympd2(const char *uri, const char *partition, const char *script) {
    sds full_uri = sdscatfmt(sdsempty(), "%s/api/%s", uri, partition);
    sds postdata = sdscatfmt(sdsempty(),
        "{\"jsonrpc\":\"2.0\",\"id\":0,\"method\":\"MYMPD_API_SCRIPT_EXECUTE\","
        "\"params\":{\"event\":\"user\",\"script\":\"%s\",\"arguments\":{}}}",
        script);
    MYGPIOD_LOG_DEBUG("Calling myMPD API \"%s\"", full_uri);
    bool rc = action_http2("POST", full_uri, "application/json", postdata);
    sdsfree(full_uri);
    sdsfree(postdata);
    return rc;
}

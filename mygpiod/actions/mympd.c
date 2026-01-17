/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/mympd.h"

#include "dist/sds/sds.h"
#include "mygpiod/actions/http.h"
#include "mygpiod/lib/log.h"

/**
 * Execute a myGPIOd script through the myGPIOd api.
 * @param cmd command to parse, format:
 *            {uri} {partition} {script}
 * @returns true on success, else false
 */
bool action_mympd(const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);
    bool rc = false;
    if (count < 3) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments (%d): \"%s\"", count, cmd);
    }
    else {
        rc = action_mympd2(args[0], args[1], args[2]);
    }
    sdsfreesplitres(args, count);
    return rc;
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

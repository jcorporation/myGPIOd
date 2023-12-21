/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/mympd.h"

#include "dist/sds/sds.h"
#include "mygpiod/actions/http.h"
#include "mygpiod/lib/log.h"

#include <curl/curl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Execute a myMPD script through the myMPD api
 * @param cmd command to parse, format:
 *            {uri} {partition} {script}
 * @returns true on success, else false
 */
bool action_mympd(const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);
    if (count < 3) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        sdsfreesplitres(args, count);
        exit(EXIT_FAILURE);
    }
    sds mympd_cmd = sdscatfmt(sdsempty(), "POST %S/api/%S application/json "
        "\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":0,\\\"method\\\":\\\"MYMPD_API_SCRIPT_EXECUTE\\\","
        "\\\"params\\\":{\\\"script\\\":\\\"%S\\\",\\\"arguments\\\":{}}}\"",
        args[0], args[1], args[2]);
    bool rc = action_http(mympd_cmd);
    sdsfree(mympd_cmd);
    return rc;
}

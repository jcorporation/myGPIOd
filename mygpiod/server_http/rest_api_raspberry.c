/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/rest_api_raspberry.h"

#include "mygpiod/lib/sds_extras.h"
#include "mygpiod/lib/json_print.h"
#include "mygpiod/raspberry/vcgencmd.h"

#include <stdlib.h>

/**
 * Handles the REST API request for GET /api/vcio/{temp,volts,clock,throttled}
 * @param buffer Already allocated buffer to populate with the response
 * @param command Command to submit
 * @param rc Pointer to bool to set the result code
 * @return Pointer to buffer
 */
sds rest_api_raspberry_vcio(sds buffer,
                            const char *command,
                            bool *rc)
{
    sds result = vcgencmd(command, sdsempty(), rc);
    if (*rc == true) {
        buffer = sdscat(buffer, "{\"value\":");
        buffer = sds_catjson(buffer, result);
        buffer = sdscatlen(buffer, "}", 1);
    }
    else {
        buffer = sdscat(buffer, "{\"error\":");
        buffer = sds_catjson(buffer, result);
        buffer = sdscatlen(buffer, "}", 1);
    }
    return buffer;
}

/**
 * Handles the REST API request for GET /api/vcio
 * @param buffer Already allocated buffer to populate with the response
 * @param rc Pointer to bool to set the result code
 * @return Pointer to buffer
 */
sds rest_api_raspberry_vcio_all(sds buffer,
                                bool *rc)
{
    const char *commands[] = {
        "measure_temp",
        "measure_volts core",
        "measure_clock arm",
        "get_throttled",
        NULL
    };
    const char *keys[] = {
        "temp",
        "volts",
        "clock",
        "throttled",
        NULL
    };
    sds result;
    buffer = sdscat(buffer, "{\"values\":{");
    const char **p = commands;
    unsigned i = 0;
    while(*p != NULL) {
        result = vcgencmd(*p, sdsempty(), rc);
        if (*rc == false) {
            break;
        }
        if (i++) {
            buffer = sdscatlen(buffer, ",", 1);
        }
        buffer = sdscatfmt(buffer, "\"%s\":", keys[i - 1]);
        buffer = sds_catjson(buffer, result);
        FREE_SDS(result);
        p++;
    }
    if (*rc == true) {
        return sdscatlen(buffer, "}}", 2);
    }
    // Failure
    sdsclear(buffer);
    buffer = sdscat(buffer, "{\"error\":");
    buffer = sds_catjson(buffer, result);
    buffer = sdscatlen(buffer, "}", 1);
    FREE_SDS(result);
    return buffer;
}

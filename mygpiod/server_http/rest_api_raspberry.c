/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/rest_api_raspberry.h"

#include "mygpiod/raspberry/vcgencmd.h"
#include "mygpiod/lib/json_print.h"

#include <errno.h>
#include <stdlib.h>

/**
 * Handles the REST API request for POST /api/gpio/{gpio_nr}/blink
 * @param config pointer to config
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds rest_api_raspberry_vcio(sds buffer,
                            const char *command,
                            bool *rc)
{
    sds result = vcgencmd(command, sdsempty(), rc);
    if (*rc == true) {
        buffer = sdscat(buffer, "{\"message\":");
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

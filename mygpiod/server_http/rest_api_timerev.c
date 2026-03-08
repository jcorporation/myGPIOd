/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief HTTP server Timer event REST API
 */

#include "compile_time.h"
#include "mygpiod/server_http/rest_api_timerev.h"

#include "mygpiod/config/timer_ev.h"
#include "mygpiod/lib/json_print.h"
#include "mygpiod/lib/timer.h"

#include <stdlib.h>

/**
 * Handles the REST API request for GET /api/v1/timerev
 * @param config pointer to config
 * @param buffer Already allocated buffer to populate with the response
 * @param rc Pointer to bool to set the result code
 * @return Pointer to buffer
 */
sds rest_api_timerev_list(struct t_config *config,
                          sds buffer,
                          bool *rc)
{
    *rc = true;
    buffer = sdscat(buffer, "{\"timers\":[");
    struct t_list_node *current = config->timer_definitions.head;
    unsigned i = 0;
    while (current != NULL) {
        struct t_timer_definition *data = (struct t_timer_definition *)current->data;
        if (i++) {
            buffer = sdscatlen(buffer, ",", 1);
        }
        buffer = sdscatfmt(buffer, "{\"name\":");
        buffer = sds_catjson(buffer, data->name);
        buffer = sdscatlen(buffer, ",", 1);
        buffer = sdscatprintf(buffer, "\"next\":%lld", (long long)timer_get_next_expire_ts(data->name, data->fd));
        buffer = sdscatlen(buffer, "}", 1);
        current = current->next;
    }
    buffer = sdscatfmt(buffer, "],\"entries\":%u}", i);
    return buffer;
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief HTTP server Timer event REST API
 */

#ifndef MYGPIOD_SERVER_HTTPD_REST_API_TIMEREV_H
#define MYGPIOD_SERVER_HTTPD_REST_API_TIMEREV_H

#include "dist/sds/sds.h"
#include "mygpiod/config/config.h"

#include <microhttpd.h>
#include <stdbool.h>

sds rest_api_timerev_list(struct t_config *config,
                          sds buffer,
                          bool *rc);

#endif

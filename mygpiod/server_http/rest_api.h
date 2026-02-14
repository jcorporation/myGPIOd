/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_REST_API_H
#define MYGPIOD_SERVER_HTTPD_REST_API_H

#include "mygpiod/config/config.h"
#include "mygpiod/server_http/util.h"

#include <microhttpd.h>

enum MHD_Result rest_api_handler(struct MHD_Connection *connection,
                                 unsigned http_conn_id,
                                 const char *url,
                                 enum http_method method,
                                 struct t_config *config);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_REST_API_H
#define MYGPIOD_SERVER_HTTPD_REST_API_H

#include "mygpiod/lib/config.h"

#include <microhttpd.h>

enum MHD_Result rest_api_handler(struct MHD_Connection *connection,
                                 const char *url,
                                 const char *method_str,
                                 const char *upload_data,
                                 struct t_config *config);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief HTTP server hooks
 */

#ifndef MYGPIOD_SERVER_HTTPD_HOOK_H
#define MYGPIOD_SERVER_HTTPD_HOOK_H

#include "mygpiod/config/config.h"

#include <microhttpd.h>

enum MHD_Result hook_handler(struct MHD_Connection *connection,
                             unsigned http_conn_id,
                             const char *url,
                             struct t_config *config);

#endif

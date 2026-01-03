/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_WEBUI_H
#define MYGPIOD_SERVER_HTTPD_WEBUI_H

#include "mygpiod/lib/config.h"

#include <microhttpd.h>

enum MHD_Result webui_handler(struct MHD_Connection *connection, const char *url);

#endif

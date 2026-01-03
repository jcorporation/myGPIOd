/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/websocket.h"

enum MHD_Result websocket_handler(struct MHD_Connection *connection, struct t_config *config) {
    (void)connection;
    (void)config;
    return MHD_NO;
}

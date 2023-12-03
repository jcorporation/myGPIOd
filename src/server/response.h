/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_RESPONSE_H
#define MYGPIOD_SERVER_RESPONSE_H

#include "src/server/socket.h"

void server_response_send(struct t_client_data *data, const char *message);
void server_response_start(struct t_client_data *data);
void server_response_append(struct t_client_data *data, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
void server_response_end(struct t_client_data *data);

#endif

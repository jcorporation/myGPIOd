/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_RESPONSE_H
#define MYGPIOD_SERVER_RESPONSE_H

#include "mygpiod/server/socket.h"

// Socket server default responses
#define DEFAULT_MSG_ERROR "ERROR:"
#define DEFAULT_MSG_OK "OK"
#define DEFAULT_MSG_END "END"

void server_response_send(struct t_client_data *client_data, const char *message);
void server_response_start(struct t_client_data *client_data);
void server_response_append(struct t_client_data *client_data, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
void server_response_end(struct t_client_data *client_data);

#endif

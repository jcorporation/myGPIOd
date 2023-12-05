/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_RESPONSE_H
#define MYGPIOD_SERVER_RESPONSE_H

#include "src/server/socket.h"

// Socket server default responses
#define DEFAULT_ERROR_MSG_PREFIX "ERROR\nmessage:"
#define DEFAULT_OK_MSG_PREFIX "OK\n"
#define DEFAULT_END_MSG "END\n"

void server_response_send(struct t_client_data *client_data, const char *message);
void server_response_start(struct t_client_data *client_data);
void server_response_append(struct t_client_data *client_data, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
void server_response_end(struct t_client_data *client_data);

#endif

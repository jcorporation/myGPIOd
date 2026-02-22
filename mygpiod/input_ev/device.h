/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event handling
 */

#ifndef MYGPIOD_INPUT_H
#define MYGPIOD_INPUT_H

#include "mygpiod/config/config.h"
#include "mygpiod/event_loop/event_loop.h"

bool input_device_open(struct t_config *config, struct t_poll_fds *poll_fds);

struct t_input_device *input_device_get_by_fd(struct t_list *inputs, int *fd);
struct t_input_device *input_device_get_by_name(struct t_list *input_devices, const char *device);

#endif

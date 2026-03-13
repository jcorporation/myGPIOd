/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myMPD (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Event handling
 */

#ifndef MYGPIOD_EVENTFD_WRAP_H
#define MYGPIOD_EVENTFD_WRAP_H

#include <stdbool.h>

int event_eventfd_create(void);
bool event_eventfd_read(int fd);
bool event_eventfd_write(int fd);
void event_fd_close(int fd);

#endif

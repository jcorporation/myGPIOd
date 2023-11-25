/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_EVENT_H
#define MYGPIOD_EVENT_H

#include <gpiod.h>
#include <stdbool.h>
#include <time.h>

struct t_mon_ctx {
    unsigned offset;
    unsigned events_wanted;
    unsigned events_done;
    bool silent;
    char *fmt;
    int sigfd;
    struct t_config *config;
};

int poll_callback(unsigned num_lines, struct gpiod_ctxless_event_poll_fd *fds, const struct timespec *timeout, void *data);
int event_callback(int event_type, unsigned line_offset, const struct timespec *timestamp, void *data);

#endif

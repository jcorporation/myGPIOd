/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <time.h>

struct t_config_node {
    unsigned gpio;
    char *cmd;
    int edge;
    int long_press;
    bool ignore_event;
    struct t_config_node *next;
};

struct t_delayed {
    int timer_fd;
    struct t_config_node *cn;
};

struct t_config {
    struct t_config_node *head;
    struct t_config_node *tail;
    unsigned length;
    int edge;
    bool active_low;
    char *bias;
    char *chip;
    int loglevel;
    time_t startup_time;
    bool syslog;
    struct t_delayed delayed_event;
};

int bias_flags(const char *option);
void config_clear(struct t_config *config);
bool config_read(struct t_config *config, const char *config_file);
struct t_config *config_new(void);

#endif

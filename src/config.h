/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_CONFIG_H
#define MYGPIOD_CONFIG_H

#include "list.h"

#include <stdbool.h>
#include <time.h>

enum gpio_values {
    GPIO_VALUE_LOW = 0,
    GPIO_VALUE_HIGH
};

struct t_gpio_node_in {
    char *cmd_rising;
    char *cmd_falling;
    int request_event;
    int fd;
    int long_press_timeout;
    char *long_press_cmd;
    int long_press_event;
    bool ignore_event;
    int timer_fd;
};

struct t_gpio_node_out {
    int value;
};

struct t_config {
    struct t_list gpios_in;
    struct t_list gpios_out;
    int event_request;
    bool active_low;
    int bias;
    char *chip_name;
    struct gpiod_chip *chip;
    int loglevel;
    bool syslog;
    int signal_fd;
    char *dir_gpio;
};

void config_clear(struct t_config *config);
bool config_read(struct t_config *config, const char *config_file);
struct t_config *config_new(void);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_CONFIG_H
#define MYGPIOD_CONFIG_H

#include <stdbool.h>
#include <time.h>

enum gpio_modes {
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT
};

enum gpio_values {
    GPIO_VALUE_UNSET = 0,
    GPIO_VALUE_LOW,
    GPIO_VALUE_HIGH
};

struct t_gpio_node {
    unsigned gpio;
    enum gpio_modes mode;
    // for inputs
    char *cmd;
    int edge;
    int long_press;
    bool ignore_event;
    // for outputs
    enum gpio_values value;
    // link to next
    struct t_gpio_node *next;
};

struct t_delayed {
    int timer_fd;
    struct t_gpio_node *cn;
};

struct t_config {
    struct t_gpio_node *gpios;
    struct t_gpio_node *gpios_tail;
    unsigned length;
    int edge;
    bool active_low;
    int bias;
    char *chip;
    int loglevel;
    time_t startup_time;
    bool syslog;
    struct t_delayed delayed_event;
};

void config_clear(struct t_config *config);
bool config_read(struct t_config *config, const char *config_file);
struct t_config *config_new(void);

#endif

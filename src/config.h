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

/**
 * Valid value of a gpio
 */
enum gpio_values {
    GPIO_VALUE_LOW = 0,
    GPIO_VALUE_HIGH
};

/**
 * Config and state data for an input gpio
 */
struct t_gpio_node_in {
    char *cmd_rising;        //!< command for rising event
    char *cmd_falling;       //!< command for falling event
    int request_event;       //!< events to request for this gpio
    int fd;                  //!< gpio file descriptor
    int long_press_timeout;  //!< timeout for the long press handler
    char *long_press_cmd;    //!< long press command
    int long_press_event;    //!< event for the long press handler
    bool ignore_event;       //!< internal state for long press handler
    int timer_fd;            //!< timer file descriptor for long press handler
};

/**
 * Config data for an output gpio
 */
struct t_gpio_node_out {
    int value;  //!< value to set
};

/**
 * Central myGPIOd config and state
 */
struct t_config {
    struct t_list gpios_in;   //!< holds the list of gpios to monitor
    struct t_list gpios_out;  //!< holds the list of gpios to set
    int event_request;        //!< events to request from the chip
    bool active_low;          //!< active state is low?
    int bias;                 //!< bias value for all the gpios in gpios_in
    char *chip_name;          //!< name / path of the gpio chip
    struct gpiod_chip *chip;  //!< gpio chip object
    int loglevel;             //!< the loglevel
    bool syslog;              //!< enable syslog?
    int signal_fd;            //!< file descriptor for the signal handler
    char *dir_gpio;           //!< directory for the gpio config files
};

void config_clear(struct t_config *config);
bool config_read(struct t_config *config, const char *config_file);
struct t_config *config_new(void);

#endif

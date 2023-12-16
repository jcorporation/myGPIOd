/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_CONFIG_H
#define MYGPIOD_CONFIG_H

#include "dist/sds/sds.h"
#include "mygpiod/lib/list.h"

#include <gpiod.h>
#include <stdbool.h>
#include <time.h>

/**
 * Config and state data for an input gpio
 */
struct t_gpio_in_data {
    enum gpiod_line_bias bias;                     //!< bias value
    bool active_low;                               //!< active state is low?
    unsigned long debounce_period_us;              //!< debounce period in microseconds
    enum gpiod_line_clock event_clock;             //!< the source clock for event timestamps
    struct t_list action_rising;                   //!< command for rising event
    struct t_list action_falling;                  //!< command for falling event
    enum gpiod_line_edge event_request;            //!< events to request for this gpio
    int gpio_fd;                                   //!< gpio file descriptor
    int long_press_timeout;                        //!< timeout for the long press handler
    struct t_list long_press_action;               //!< long press command
    enum gpiod_line_edge long_press_event;         //!< event for the long press handler
    bool ignore_event;                             //!< internal state for long press handler
    int timer_fd;                                  //!< timer file descriptor for long press handler
    struct gpiod_edge_event_buffer *event_buffer;  //!< buffer for gpio events
    struct gpiod_line_request *request;            //!< gpio line request struct
};

/**
 * Config data for an output gpio
 */
struct t_gpio_out_data {
    enum gpiod_line_drive drive;         //!< drive value
    enum gpiod_line_value value;         //!< value to set
    struct gpiod_line_request *request;  //!< gpio line request struct
};

/**
 * Central myGPIOd config and state
 */
struct t_config {
    struct t_list gpios_in;   //!< holds the list of gpios to monitor
    struct t_list gpios_out;  //!< holds the list of gpios to set
    sds chip_path;            //!< path of the gpio chip device
    int loglevel;             //!< the loglevel
    bool syslog;              //!< enable syslog?
    int signal_fd;            //!< file descriptor for the signal handler
    sds dir_gpio;             //!< directory for the gpio config files

    sds socket_path;          //!< server socket
    int socket_timeout;       //!< socket timeout in seconds
    struct t_list clients;    //!< list of connected clients
    unsigned client_id;       //!< uniq client id

    struct gpiod_chip *chip;  //!< gpiod chip object
};

void config_clear(struct t_config *config);
struct t_config *get_config(sds config_file);

#endif

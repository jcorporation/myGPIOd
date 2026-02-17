/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief GPIO configuration
 */

#ifndef MYGPIOD_CONFIG_GPIO_H
#define MYGPIOD_CONFIG_GPIO_H

#include "dist/sds/sds.h"
#include "mygpiod/lib/list.h"

#include <gpiod.h>
#include <stdbool.h>

/**
 * Config and state data for an input gpio
 */
struct t_gpio_in_data {
    enum gpiod_line_bias bias;                     //!< bias value
    bool active_low;                               //!< active state is low?
    unsigned long debounce_period_us;              //!< debounce period in microseconds
    enum gpiod_line_clock event_clock;             //!< the source clock for event timestamps
    struct t_list action_rising;                   //!< list of actions for rising event
    struct t_list action_falling;                  //!< list of actions for falling event
    enum gpiod_line_edge event_request;            //!< events to request for this gpio
    int gpio_fd;                                   //!< gpio file descriptor
    int long_press_timeout_ms;                     //!< timeout for the long press handler in milliseconds
    int long_press_interval_ms;                    //!< interval for the long press handler in milliseconds
    struct t_list long_press_action;               //!< list of actions for long press
    struct t_list long_press_release_action;       //!< list of actions for long press release
    enum gpiod_line_edge long_press_event;         //!< event for the long press handler
    enum gpiod_line_value long_press_value;        //!< initial gpio value for the long press event
    bool ignore_event;                             //!< internal state for long press handler
    int timer_fd;                                  //!< timer file descriptor for long press handler
    struct gpiod_edge_event_buffer *event_buffer;  //!< buffer for gpio events
    struct gpiod_line_request *request;            //!< gpio line request struct
    sds name;                                      //!< gpio name
};

/**
 * Config data for an output gpio
 */
struct t_gpio_out_data {
    enum gpiod_line_drive drive;         //!< drive value
    enum gpiod_line_value value;         //!< value to set
    int timer_fd;                        //!< timer file descriptor for blink handler
    struct gpiod_line_request *request;  //!< gpio line request struct
    sds name;                            //!< gpio name
};


bool parse_gpio_config_file(int direction, void *data, const char *dirname, const char *filename);
bool parse_gpio_config_file_in_kv(sds key, sds value, struct t_gpio_in_data *data);
bool parse_gpio_config_file_out_kv(sds key, sds value, struct t_gpio_out_data *data);
struct t_gpio_in_data *gpio_in_data_new(void);
struct t_gpio_out_data *gpio_out_data_new(void);
void gpio_in_data_clear(struct t_gpio_in_data *data);
void gpio_node_in_clear(struct t_list_node *node);
void gpio_out_data_clear(struct t_gpio_out_data *data);
void gpio_node_out_clear(struct t_list_node *node);

#endif

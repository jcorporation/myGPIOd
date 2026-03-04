/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Global configuration
 */

#ifndef MYGPIOD_CONFIG_H
#define MYGPIOD_CONFIG_H

#include "dist/sds/sds.h"
#include "mygpiod/lib/list.h"

#include <stdbool.h>

#ifdef MYGPIOD_ENABLE_ACTION_MPC
    #include <mpd/client.h>
#endif

#ifdef MYGPIOD_ENABLE_ACTION_LUA
    #include <lua.h>
#endif

/**
 * Central myGPIOd config and state
 */
struct t_config {
    // Configuration
    int loglevel;                         //!< The loglevel
    bool syslog;                          //!< Enable syslog?
    int signal_fd;                        //!< File descriptor for the signal handler

    // Socket Server
    sds socket_path;                      //!< Server socket filepath
    int socket_timeout_s;                 //!< Socket timeout in seconds
    struct t_list clients;                //!< List of connected socket clients
    unsigned client_id;                   //!< Uniq client id

    #ifdef MYGPIOD_ENABLE_HTTPD
        // HTTP Server
        struct MHD_Daemon *httpd;         //!< HTTPD object
        sds http_ip;                      //!< HTTPD listening ip
        unsigned http_port;               //!< HTTPD listening port
        struct t_list http_suspended;     //!< List of suspended HTTP connections
        unsigned http_conn_id;            //!< Uniq HTTP connection id
    #endif

    // GPIO
    sds dir_gpio;                         //!< Directory for the gpio config files
    struct t_list gpios_in;               //!< List of GPIOs to monitor
    struct t_list gpios_out;              //!< List of GPIOs to set
    sds chip_path;                        //!< Path of the gpio chip device
    struct gpiod_chip *chip;              //!< Gpiod chip object

    // Input events
    struct t_list input_devices;          //!< List of /dev/input/* devices

    // Timer events
    struct t_list timer_definitions;      //!< List of timer configurations

    // Timer events
    struct t_list hooks;                 //!< List of hooks

    // MPD
    #ifdef MYGPIOD_ENABLE_ACTION_MPC
        struct mpd_connection *mpd_conn;  //!< MPD connection
    #endif

    // Lua
    #ifdef MYGPIOD_ENABLE_ACTION_LUA
        sds lua_file;                     //!< Lua file to load
        lua_State* lua_vm;                //!< Lua VM
    #endif
};

void config_clear(struct t_config *config);
struct t_config *get_config(sds config_file);

#endif

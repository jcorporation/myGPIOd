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
    int loglevel;                         //!< the loglevel
    bool syslog;                          //!< enable syslog?
    int signal_fd;                        //!< file descriptor for the signal handler

    // Socket Server
    sds socket_path;                      //!< server socket filepath
    int socket_timeout_s;                 //!< socket timeout in seconds
    struct t_list clients;                //!< list of connected socket clients
    unsigned client_id;                   //!< uniq client id

    #ifdef MYGPIOD_ENABLE_HTTPD
        // HTTP Server
        struct MHD_Daemon *httpd;             //!< HTTPD object
        sds http_ip;                          //!< HTTPD listening ip
        unsigned http_port;                   //!< HTTPD listening port
        struct t_list http_suspended;         //!< List of suspended HTTP connections
        unsigned http_conn_id;                //!< Uniq HTTP connection id
    #endif

    // GPIO
    sds dir_gpio;                         //!< directory for the gpio config files
    struct t_list gpios_in;               //!< holds the list of GPIOs to monitor
    struct t_list gpios_out;              //!< holds the list of GPIOs to set
    sds chip_path;                        //!< path of the gpio chip device
    struct gpiod_chip *chip;              //!< gpiod chip object

    // input events
    struct t_list input_devices;                 //!< list of /dev/input/* devices

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

void config_gpios_clear(struct t_config *config);
void config_clear(struct t_config *config);
struct t_config *get_config(sds config_file);

#endif

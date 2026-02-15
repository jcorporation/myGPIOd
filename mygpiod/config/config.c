/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Global configuration
 */

#include "compile_time.h"
#include "mygpiod/config/config.h"

#include "dist/sds/sds.h"
#include "mygpio-common/util.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/event_loop/signal_handler.h"

#include "mygpiod/config/gpio.h"
#include "mygpiod/config/input.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/sds_extras.h"
#include "mygpiod/server_http/util.h"
#include "mygpiod/server_socket/socket.h"

#include <dirent.h>
#include <errno.h>
#include <gpiod.h>
#include <limits.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//private definitions
static bool config_read(struct t_config *config, sds config_file);
static struct t_config *config_new(void);
static bool parse_config_file_kv(sds key, sds value, struct t_config *config);

//public functions

/**
 * Allocates a config struct and reads the configuration file
 * @param config_file configuration file name
 * @return populated config struct or NULL on error
 */
struct t_config *get_config(sds config_file) {
    MYGPIOD_LOG_INFO("Reading \"%s\"", config_file);
    struct t_config *config = config_new();
    if (config == NULL) {
        MYGPIOD_LOG_EMERG("Out of memory");
        return NULL;
    }
    if (config_read(config, config_file) == false) {
        config_clear(config);
        FREE_PTR(config);
        return NULL;
    }
    return config;
}

/**
 * Clears the gpio part of the config
 * @param config pointer to config
 */
void config_gpios_clear(struct t_config *config) {
    list_clear(&config->gpios_in, gpio_node_in_clear);
    list_clear(&config->gpios_out, gpio_node_out_clear);
}

/**
 * Clears the config struct.
 * @param config pointer to config to free
 */
void config_clear(struct t_config *config) {
    config_gpios_clear(config);
    list_clear(&config->clients, server_client_connection_clear);
    if (config->chip != NULL) {
        gpiod_chip_close(config->chip);
    }
    close_fd(&config->signal_fd);
    FREE_SDS(config->chip_path);
    FREE_SDS(config->dir_gpio);
    FREE_SDS(config->socket_path);
    FREE_SDS(config->http_ip);
    #ifdef MYGPIOD_ENABLE_ACTION_MPC
        if (config->mpd_conn != NULL) {
            mpd_connection_free(config->mpd_conn);
        }
    #endif
    #ifdef MYGPIOD_ENABLE_ACTION_LUA
        if (config->lua_vm != NULL) {
            lua_close(config->lua_vm);
        }
        FREE_SDS(config->lua_file);
    #endif
    if (config->httpd != NULL) {
        struct t_list_node *current = config->http_suspended.head;
        while (current != NULL) {
            struct t_request_data *request_data = (struct t_request_data *)current->data;
            MHD_resume_connection(request_data->connection);
            current = current->next;
        }
        MHD_stop_daemon(config->httpd);
        list_clear(&config->http_suspended, NULL);
    }
    list_clear(&config->inputs, input_node_data_clear);
}

//private functions

/**
 * Allocates a new config struct and sets defaults
 * @return allocated struct with default values assigned.
 */
static struct t_config *config_new(void) {
    struct t_config *config = malloc_assert(sizeof(struct t_config));
    config->signal_fd = make_signalfd();
    if (config->signal_fd == -1) {
        FREE_PTR(config);
        return NULL;
    }
    list_init(&config->gpios_in);
    list_init(&config->gpios_out);
    config->chip_path = sdsempty();
    config->chip = NULL;
    config->loglevel = loglevel;
    config->syslog = CFG_SYSLOG;
    config->dir_gpio = sdsnew(CFG_GPIO_DIR);
    config->socket_path = sdsnew(CFG_SOCKET_PATH);
    config->socket_timeout_s = CFG_SOCKET_TIMEOUT;
    config->client_id = 0;
    list_init(&config->clients);
    #ifdef MYGPIOD_ENABLE_ACTION_MPC
        config->mpd_conn = NULL;
    #endif
    #ifdef MYGPIOD_ENABLE_ACTION_LUA
        config->lua_vm = NULL;
        config->lua_file = sdsempty();
    #endif

    list_init(&config->inputs);

    config->http_ip = sdsnew(CFG_HTTP_IP);
    config->http_port = CFG_HTTP_PORT;
    config->httpd = NULL;
    list_init(&config->http_suspended);
    config->http_conn_id = 0;
    return config;
}

/**
 * Reads the mygpiod config file
 * @param config config struct to populate
 * @param config_file config filename to read
 * @return true on success, else false
 */
static bool config_read(struct t_config *config, sds config_file) {
    FILE *fp = fopen(config_file, OPEN_FLAGS_READ);
    if (fp == NULL) {
        MYGPIOD_LOG_ERROR("Can not open \"%s\": %s", config_file, strerror(errno));
        return false;
    }
    bool rc = true;
    sds line = sdsempty();
    unsigned line_num = 0;
    int nread = 0;
    while ((line = sds_getline(line, fp, LINE_LENGTH_MAX, &nread)) && nread >= 0) {
        line_num++;
        //ignore blank lines and comments
        if (line[0] == '#' ||
            sdslen(line) == 0)
        {
            continue;
        }
        MYGPIOD_LOG_DEBUG("Parsing line: \"%s\"", line);
        int count = 0;
        sds *kv = sds_splitfirst(line, '=', &count);
        if (count == 2) {
            if (parse_config_file_kv(kv[0], kv[1], config) == false) {
                MYGPIOD_LOG_EMERG("Invalid config line #%u", line_num);
                sdsfreesplitres(kv, count);
                rc = false;
                break;
            }
        }
        sdsfreesplitres(kv, count);
    }
    FREE_SDS(line);
    (void)fclose(fp);

    // exit on on invalid config
    if (rc == false) {
        return false;
    }

    //gpio config
    errno = 0;
    DIR *dir = opendir(config->dir_gpio);
    if (dir == NULL) {
        MYGPIOD_LOG_ERROR("Error opening directory \"%s\"", config->dir_gpio);
        return false;
    }
    unsigned i = 0;
    struct dirent *next_file;
    while ((next_file = readdir(dir)) != NULL ) {
        if (next_file->d_type != DT_REG) {
            continue;
        }
        unsigned gpio;
        char *rest;
        if (mygpio_parse_uint(next_file->d_name, &gpio, &rest, 0, 99) == true) {
            if (rest[0] == '.') {
                rest++;
                if (strcmp(rest, "in") == 0) {
                    MYGPIOD_LOG_DEBUG("Parsing %s/%s", config->dir_gpio, next_file->d_name);
                    struct t_gpio_in_data *data = gpio_in_data_new();
                    rc = parse_gpio_config_file(GPIOD_LINE_DIRECTION_INPUT, data, config->dir_gpio, next_file->d_name);
                    if (rc == true && list_push(&config->gpios_in, gpio, data) == true) {
                        i++;
                        continue;
                    }
                    gpio_in_data_clear(data);
                    FREE_PTR(data);
                }
                else if (strcmp(rest, "out") == 0) {
                    MYGPIOD_LOG_DEBUG("Parsing %s/%s", config->dir_gpio, next_file->d_name);
                    struct t_gpio_out_data *data = gpio_out_data_new();
                    rc = parse_gpio_config_file(GPIOD_LINE_DIRECTION_OUTPUT, data, config->dir_gpio, next_file->d_name);
                    if (rc == true && list_push(&config->gpios_out, gpio, data) == true) {
                        i++;
                        continue;
                    }
                    gpio_out_data_clear(data);
                    FREE_PTR(data);
                }
            }
        }
        MYGPIOD_LOG_WARN("Skipping file %s/%s", config->dir_gpio, next_file->d_name);
        if (i == GPIOS_MAX) {
            MYGPIOD_LOG_WARN("Too many gpios configured");
            break;
        }
    }
    closedir(dir);
    MYGPIOD_LOG_INFO("Parsed %u gpio config files", i);
    return true;
}

/**
 * Parses a configuration line
 * @param key configuration key
 * @param value configuration value
 * @param config already allocated config struct to populate
 * @return true on success, else false
 */
static bool parse_config_file_kv(sds key, sds value, struct t_config *config) {
    errno = 0;
    if (strcmp(key, "chip") == 0) {
        sdsclear(config->chip_path);
        config->chip_path = sdscatsds(config->chip_path, value);
        MYGPIOD_LOG_DEBUG("Setting chip to \"%s\"", config->chip_path);
        return true;
    }
    if (strcmp(key, "loglevel") == 0) {
        config->loglevel = parse_loglevel(value);
        MYGPIOD_LOG_DEBUG("Setting loglevel to \"%s\"", lookup_loglevel(config->loglevel));
        return errno == 0 ? true : false;
    }
    if (strcmp(key, "syslog") == 0) {
        config->syslog = mygpio_parse_bool(value);
        MYGPIOD_LOG_DEBUG("Setting syslog to \"%s\"", mygpio_bool_to_str(config->syslog));
        return errno == 0 ? true : false;
    }
    if (strcmp(key, "gpio_dir") == 0) {
        sdsclear(config->dir_gpio);
        config->dir_gpio = sdscat(config->dir_gpio, value);
        MYGPIOD_LOG_DEBUG("Setting gpio_dir to \"%s\"", config->dir_gpio);
        return true;
    }
    if (strcmp(key, "socket") == 0) {
        if (sdslen(value) >= 108) {
            MYGPIOD_LOG_WARN("Socket path is too long");
            return false;
        }
        sdsclear(config->socket_path);
        config->socket_path = sdscatsds(config->socket_path, value);
        MYGPIOD_LOG_DEBUG("Setting socket to \"%s\"", config->dir_gpio);
        return true;
    }
    if (strcmp(key, "timeout") == 0) {
        if (mygpio_parse_int(value, &config->socket_timeout_s, NULL, 10, 120) == true) {
            MYGPIOD_LOG_DEBUG("Setting timeout to \"%d\" seconds", config->socket_timeout_s);
            return true;
        }
        return false;
    }
    if (strcmp(key, "http_ip") == 0) {
        sdsclear(config->http_ip);
        config->http_ip = sdscat(config->http_ip, value);
        MYGPIOD_LOG_DEBUG("Setting http_ip to \"%s\"", config->http_ip);
        return true;
    }
    if (strcmp(key, "http_port") == 0) {
        if (mygpio_parse_uint(value, &config->http_port, NULL, 1025, 65535) == true) {
            MYGPIOD_LOG_DEBUG("Setting http port to \"%u\" seconds", config->http_port);
            return true;
        }
        return false;
    }
    if (strcmp(key, "input") == 0) {
        struct t_input_device *data = malloc_assert(sizeof(struct t_input_device));
        data->fd = -1;
        data->name = sdsdup(value);
        list_init(&data->event_actions);
        list_push(&config->inputs, 0, data);
        MYGPIOD_LOG_DEBUG("Adding input %s", value);
        return true;
    }
    if (strcmp(key, "input_ev") == 0) {
        if (parse_input_ev(config, value) == true) {
            return true;
        }
        return false;
    }
    #ifdef MYGPIOD_ENABLE_ACTION_LUA
        if (strcmp(key, "lua_file") == 0) {
            config->lua_file = sdscatsds(config->lua_file, value);
            return true;
        }
    #endif
    return false;
}

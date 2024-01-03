/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/lib/config.h"

#include "mygpio-common/util.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/event_loop/signal_handler.h"
#include "mygpiod/lib/action.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/util.h"
#include "mygpiod/server/socket.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <gpiod.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//private definitions
static bool config_read(struct t_config *config, sds config_file);
static struct t_config *config_new(void);
static bool parse_config_file_kv(sds key, sds value, struct t_config *config);
static bool parse_gpio_config_file(int direction, void *data, const char *dirname, const char *filename);
static bool parse_gpio_config_file_in_kv(sds key, sds value, struct t_gpio_in_data *data);
static bool parse_gpio_config_file_out_kv(sds key, sds value, struct t_gpio_out_data *data);
static struct t_action *action_node_data_from_value(sds value);
static struct t_gpio_in_data *gpio_in_data_new(void);
static struct t_gpio_out_data *gpio_out_data_new(void);
static void gpio_in_data_clear(struct t_gpio_in_data *data);
static void gpio_node_in_clear(struct t_list_node *node);
static void gpio_out_data_clear(struct t_gpio_out_data *data);
static void gpio_node_out_clear(struct t_list_node *node);

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
 * Clears the config struct.
 * @param config pointer to config to free
 */
void config_clear(struct t_config *config) {
    list_clear(&config->gpios_in, gpio_node_in_clear);
    list_clear(&config->gpios_out, gpio_node_out_clear);
    list_clear(&config->clients, server_client_connection_clear);
    if (config->chip != NULL) {
        gpiod_chip_close(config->chip);
    }
    close_fd(&config->signal_fd);
    FREE_SDS(config->chip_path);
    FREE_SDS(config->dir_gpio);
    FREE_SDS(config->socket_path);
    #ifdef MYGPIOD_ENABLE_ACTION_MPC
        if (config->mpd_conn != NULL) {
            mpd_connection_free(config->mpd_conn);
        }
    #endif
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
    config->chip_path = sdsnew(CFG_CHIP);
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
                MYGPIOD_LOG_WARN("Invalid config line #%u", line_num);
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
    return false;
}

/**
 * Parses a gpio configuration file
 * @param direction one off GPIOD_LINE_DIRECTION_INPUT, GPIOD_LINE_DIRECTION_OUTPUT
 * @param data already allocated gpio config data to populate
 * @param dirname directory name
 * @param filename file name
 * @return true on success, else false
 */
static bool parse_gpio_config_file(int direction, void *data, const char *dirname, const char *filename) {
    sds filepath = sdscatprintf(sdsempty(), "%s/%s", dirname, filename);
    FILE *fp = fopen(filepath, OPEN_FLAGS_READ);
    if (fp == NULL) {
        MYGPIOD_LOG_ERROR("Error opening \"%s\"", filepath);
        FREE_SDS(filepath);
        return false;
    }
    FREE_SDS(filepath);
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
            bool rc = direction == GPIOD_LINE_DIRECTION_OUTPUT
                ? parse_gpio_config_file_out_kv(kv[0], kv[1], data)
                : parse_gpio_config_file_in_kv(kv[0], kv[1], data);
            if (rc == false) {
                MYGPIOD_LOG_WARN("Invalid config line %s#%u", filename, line_num);
            }
        }
        sdsfreesplitres(kv, count);
    }
    FREE_SDS(line);
    (void)fclose(fp);
    return true;
}

/**
 * Parses a key/value pair from a gpio out configuration file
 * @param key configuration key
 * @param value configuration value
 * @param data already allocated gpio config data to populate
 * @return true on success, else false
 */
static bool parse_gpio_config_file_out_kv(sds key, sds value, struct t_gpio_out_data *data) {
    errno = 0;
    if (strcmp(key, "drive") == 0) {
        data->drive = parse_drive(value);
        return errno == 0 ? true : false;
    }
    if (strcmp(key, "value") == 0) {
        data->value = parse_gpio_value(value);
        return errno == 0 ? true : false;
    }
    return false;
}

/**
 * Parses a key/value pair from a gpio in configuration file
 * @param key configuration key
 * @param value configuration value
 * @param data already allocated gpio config data to populate
 * @return true on success, else false
 */
static bool parse_gpio_config_file_in_kv(sds key, sds value, struct t_gpio_in_data *data) {
    errno = 0;
    if (strcmp(key, "active_low") == 0) {
        data->active_low = mygpio_parse_bool(value);
        return errno == 0 ? true : false;
    }
    if (strcmp(key, "bias") == 0) {
        data->bias = parse_bias(value);
        return errno == 0 ? true : false;
    }
    if (strcmp(key, "event_request") == 0) {
        data->event_request = parse_event_request(value);
        return errno == 0 ? true : false;
    }
    if (strcmp(key, "debounce") == 0) {
        if (mygpio_parse_ulong(value, &data->debounce_period_us, NULL, 0, UINT_MAX) == true) {
            return errno == 0 ? true : false;
        }
        return false;
    }
    if (strcmp(key, "event_clock") == 0) {
        data->event_clock = parse_event_clock(value);
        return errno == 0 ? true : false;
    }
    if (strcmp(key, "action_falling") == 0) {
        struct t_action *action_data = action_node_data_from_value(value);
        if (action_data != NULL) {
            return list_push(&data->action_falling, data->action_falling.length, action_data);
        }
        return false;
    }
    if (strcmp(key, "action_rising") == 0) {
        struct t_action *action_data = action_node_data_from_value(value);
        if (action_data != NULL) {
            return list_push(&data->action_rising, data->action_rising.length, action_data);
        }
        return false;
    }
    if (strcmp(key, "long_press_timeout") == 0) {
        if (mygpio_parse_int(value, &data->long_press_timeout_ms, NULL, 0, TIMEOUT_MS_MAX) == true) {
            return errno == 0 ? true : false;
        }
        return false;
    }
    if (strcmp(key, "long_press_interval") == 0) {
        if (mygpio_parse_int(value, &data->long_press_interval_ms, NULL, 0, TIMEOUT_MS_MAX) == true) {
            return errno == 0 ? true : false;
        }
        return false;
    }
    if (strcmp(key, "long_press_event") == 0) {
        data->long_press_event = parse_event_request(value);
        return errno == 0 ? true : false;
    }
    if (strcmp(key, "long_press_action") == 0) {
        struct t_action *action_data = action_node_data_from_value(value);
        if (action_data != NULL) {
            return list_push(&data->long_press_action, data->long_press_action.length, action_data);
        }
        return false;
    }
    if (strcmp(key, "long_press_release_action") == 0) {
        struct t_action *action_data = action_node_data_from_value(value);
        if (action_data != NULL) {
            return list_push(&data->long_press_release_action, data->long_press_release_action.length, action_data);
        }
        return false;
    }
    return false;
}

/**
 * Splits the action from the action option string.
 * @param value string to split.
 * @return allocated t_action struct or NULL on error.
 */
static struct t_action *action_node_data_from_value(sds value) {
    int count = 0;
    sds *kv = sds_splitfirst(value, ':', &count);
    struct t_action *data = NULL;
    if (count == 2) {
        enum mygpiod_actions action = parse_action(kv[0]);
        if (action != MYGPIOD_ACTION_UNKNOWN) {
            data = action_node_data_new(action, kv[1]);
        }
        else {
            MYGPIOD_LOG_WARN("Invalid action: %s", value);
        }
    }
    sdsfreesplitres(kv, count);
    return data;
}

/**
 * Creates a new gpio in config data node and sets its values to defaults.
 * @return newly allocated struct t_gpio_node_in
 */
static struct t_gpio_in_data *gpio_in_data_new(void) {
    struct t_gpio_in_data *data = malloc_assert(sizeof(struct t_gpio_in_data));
    data->event_request = GPIOD_LINE_EDGE_RISING;
    list_init(&data->action_rising);
    list_init(&data->action_falling);
    data->long_press_timeout_ms = 0;
    data->long_press_interval_ms = 0;
    list_init(&data->long_press_action);
    list_init(&data->long_press_release_action);
    data->long_press_event = GPIOD_LINE_EDGE_FALLING;
    data->long_press_value = GPIOD_LINE_VALUE_ERROR;
    data->ignore_event = false;
    data->timer_fd = -1;
    data->gpio_fd = -1;
    data->bias = GPIOD_LINE_BIAS_AS_IS;
    data->active_low = false;
    data->debounce_period_us = 0;
    data->event_clock = GPIOD_LINE_CLOCK_REALTIME;
    data->request = NULL;
    data->event_buffer = NULL;
    return data;
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param data gpio in config node to clear
 */
static void gpio_in_data_clear(struct t_gpio_in_data *data) {
    close_fd(&data->gpio_fd);
    close_fd(&data->timer_fd);
    if (data->request != NULL) {
        gpiod_line_request_release(data->request);
    }
    if (data->event_buffer != NULL) {
        gpiod_edge_event_buffer_free(data->event_buffer);
    }
    list_clear(&data->action_falling, node_data_action_clear);
    list_clear(&data->action_rising, node_data_action_clear);
    list_clear(&data->long_press_action, node_data_action_clear);
    list_clear(&data->long_press_release_action, node_data_action_clear);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node gpio in config node to clear
 */
static void gpio_node_in_clear(struct t_list_node *node) {
    struct t_gpio_in_data *data = (struct t_gpio_in_data *)node->data;
    gpio_in_data_clear(data);
}

/**
 * Creates a new gpio out config data node and sets its values to defaults.
 * @return newly allocated struct t_gpio_out_data
 */
static struct t_gpio_out_data *gpio_out_data_new(void) {
    struct t_gpio_out_data *data = malloc_assert(sizeof(struct t_gpio_out_data));
    data->drive = GPIOD_LINE_DRIVE_PUSH_PULL;
    data->value = GPIOD_LINE_VALUE_INACTIVE;
    data->timer_fd = -1;
    data->request = NULL;
    return data;
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param data gpio out data to clear
 */
static void gpio_out_data_clear(struct t_gpio_out_data *data) {
    close_fd(&data->timer_fd);
    if (data->request != NULL) {
        gpiod_line_request_release(data->request);
    }
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node gpio out config node to clear
 */
static void gpio_node_out_clear(struct t_list_node *node) {
    struct t_gpio_out_data *data = (struct t_gpio_out_data *)node->data;
    gpio_out_data_clear(data);
}

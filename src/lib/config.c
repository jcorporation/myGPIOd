/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "src/lib/config.h"

#include "src/event_loop/signal_handler.h"
#include "src/lib/list.h"
#include "src/lib/log.h"
#include "src/lib/mem.h"
#include "src/lib/util.h"
#include "src/server/socket.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//private definitions
static bool config_read(struct t_config *config, sds config_file);
static struct t_config *config_new(void);
static bool parse_config_file_kv(sds key, sds value, struct t_config *config);
static bool parse_gpio_config_file(int mode, void *data, const char *dirname, const char *filename);
static bool parse_gpio_config_file_in_kv(sds key, sds value, struct t_gpio_in_data *data);
static bool parse_gpio_config_file_out_kv(sds key, sds value, struct t_gpio_out_data *data);
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
    if (config->chip != NULL) {
        gpiod_chip_close(config->chip);
    }
    list_clear(&config->gpios_in, gpio_node_in_clear);
    list_clear(&config->gpios_out, gpio_node_out_clear);
    list_clear(&config->clients, server_client_connection_clear);
    close_fd(&config->signal_fd);
    FREE_SDS(config->chip_name);
    FREE_SDS(config->dir_gpio);
    FREE_SDS(config->socket_path);
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
    config->chip_name = sdsnew(CFG_CHIP);
    config->chip = NULL;
    config->bulk_in = NULL;
    config->active_low = CFG_ACTIVE_LOW;
    config->bias = CFG_BIAS;
    config->event_request = CFG_REQUEST_EVENT_CHIP;
    config->loglevel = loglevel;
    config->syslog = CFG_SYSLOG;
    config->dir_gpio = sdsnew(CFG_GPIO_DIR);
    config->socket_path = sdsnew(CFG_SOCKET_PATH);
    config->socket_timeout = CFG_SOCKET_TIMEOUT;
    config->client_id = 0;
    list_init(&config->clients);
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
    
    sds line = sdsempty();
    unsigned line_num = 0;
    while (sds_getline(&line, fp, LINE_LENGTH_MAX) >= 0) {
        line_num++;
        //ignore blank lines and comments
        if (line[0] == '#' ||
            sdslen(line) == 0)
        {
            continue;
        }
        int count = 0;
        sds *kv = sdssplitlen(line, (ssize_t)sdslen(line), "=", 1, &count);
        if (count == 2) {
            sdstrim(kv[0], " \t");
            sdstrim(kv[1], " \t");
            if (parse_config_file_kv(kv[0], kv[1], config) == false) {
                MYGPIOD_LOG_WARN("Invalid config line #%u", line_num);
            }
        }
        sdsfreesplitres(kv, count);
    }
    FREE_SDS(line);
    (void)fclose(fp);

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
        if (parse_uint(next_file->d_name, &gpio, &rest, 0, 99) == true) {
            if (rest[0] == '.') {
                rest++;
                if (strcmp(rest, "in") == 0) {
                    struct t_gpio_in_data *data = gpio_in_data_new();
                    bool rc = parse_gpio_config_file(GPIOD_LINE_DIRECTION_INPUT, data, config->dir_gpio, next_file->d_name);
                    if (rc == true && list_push(&config->gpios_in, gpio, data) == true) {
                        i++;
                        continue;
                    }
                    gpio_in_data_clear(data);
                    FREE_PTR(data);
                }
                else if (strcmp(rest, "out") == 0) {
                    struct t_gpio_out_data *data = gpio_out_data_new();
                    bool rc = parse_gpio_config_file(GPIOD_LINE_DIRECTION_OUTPUT, data, config->dir_gpio, next_file->d_name);
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
        if (i == GPIOD_LINE_BULK_MAX_LINES) {
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
    if (strcmp(key, "chip") == 0) {
        sdsclear(config->chip_name);
        config->chip_name = sdscatsds(config->chip_name, value);
        MYGPIOD_LOG_DEBUG("Setting chip to \"%s\"", config->chip_name);
        return true;
    }
    if (strcmp(key, "request_event") == 0) {
        config->event_request = parse_event_request(value);
        MYGPIOD_LOG_DEBUG("Setting event to \"%s\"", lookup_event_request(config->event_request));
        return true;
    }
    if (strcmp(key, "active_low") == 0) {
        config->active_low = parse_bool(value);
        MYGPIOD_LOG_DEBUG("Setting active_low to \"%s\"", bool_to_str(config->active_low));
        return true;
    }
    if (strcmp(key, "bias") == 0) {
        config->bias = parse_bias(value);
        MYGPIOD_LOG_DEBUG("Setting bias to \"%s\"", lookup_bias(config->bias));
        return true;
    }
    if (strcmp(key, "loglevel") == 0) {
        config->loglevel = parse_loglevel(value);
        MYGPIOD_LOG_DEBUG("Setting loglevel to \"%s\"", lookup_loglevel(config->loglevel));
        return true;
    }
    if (strcmp(key, "syslog") == 0) {
        config->syslog = parse_bool(value);
        MYGPIOD_LOG_DEBUG("Setting syslog to \"%s\"", bool_to_str(config->syslog));
        return true;
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
        parse_int(value, &config->socket_timeout, NULL, 10, 120);
        MYGPIOD_LOG_DEBUG("Setting socket to \"%s\"", config->dir_gpio);
        return true;
    }
    return false;
}

/**
 * Parses a gpio configuration file
 * @param mode one off GPIOD_LINE_DIRECTION_INPUT, GPIOD_LINE_DIRECTION_OUTPUT
 * @param data already allocated gpio config data to populate
 * @param dirname directory name
 * @param filename file name
 * @return true on success, else false
 */
static bool parse_gpio_config_file(int mode, void *data, const char *dirname, const char *filename) {
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
    while (sds_getline(&line, fp, LINE_LENGTH_MAX) >= 0) {
        line_num++;
        //ignore blank lines and comments
        if (line[0] == '#' ||
            sdslen(line) == 0)
        {
            continue;
        }
        int count = 0;
        sds *kv = sdssplitlen(line, (ssize_t)sdslen(line), "=", 1, &count);
        if (count == 2) {
            sdstrim(kv[0], " \t");
            sdstrim(kv[1], " \t");
            bool rc = mode == GPIOD_LINE_DIRECTION_OUTPUT
                ? parse_gpio_config_file_out_kv(kv[0], kv[1], data)
                : parse_gpio_config_file_in_kv(kv[0], kv[1], data);
            if (rc == false) {
                MYGPIOD_LOG_WARN("Invalid config line #%u", line_num);
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
    if (strcmp(key, "value") == 0) {
        data->value = parse_gpio_value(value);
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
    if (strcmp(key, "request_event") == 0) {
        data->request_event = parse_event_request(value);
        return true;
    }
    if (strcmp(key, "action_falling") == 0) {
        sdsclear(data->action_falling);
        data->action_falling = sdscatsds(data->action_falling, value);
        return true;
    }
    if (strcmp(key, "action_rising") == 0) {
        sdsclear(data->action_rising);
        data->action_rising = sdscatsds(data->action_rising, value);
        return true;
    }
    if (strcmp(key, "long_press_timeout") == 0) {
        if (parse_int(value, &data->long_press_timeout, NULL, 0, 9) == true) {
            return true;
        }
    }
    if (strcmp(key, "long_press_event") == 0) {
        data->long_press_event = parse_event_request(key);
        return true;
    }
    if (strcmp(key, "long_press_action") == 0) {
        sdsclear(data->long_press_action);
        data->long_press_action = sdscatsds(data->long_press_action, value);
        return true;
    }
    return false;
}

/**
 * Creates a new gpio in config data node and sets its values to defaults.
 * @return newly allocated struct t_gpio_node_in
 */
static struct t_gpio_in_data *gpio_in_data_new(void) {
    struct t_gpio_in_data *data = malloc_assert(sizeof(struct t_gpio_in_data));
    data->request_event = GPIOD_LINE_REQUEST_EVENT_RISING_EDGE;
    data->action_rising = sdsempty();
    data->action_falling = sdsempty();
    data->long_press_timeout = 0;
    data->long_press_action = sdsempty();
    data->long_press_event = GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE;
    data->ignore_event = false;
    data->timer_fd = -1;
    data->fd = -1;
    return data;
}

/**
 * Creates a new gpio out config data node and sets its values to defaults.
 * @return newly allocated struct t_gpio_out_data
 */
static struct t_gpio_out_data *gpio_out_data_new(void) {
    struct t_gpio_out_data *data = malloc_assert(sizeof(struct t_gpio_out_data));
    data->value = GPIO_VALUE_LOW;
    return data;
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param data gpio in config node to clear
 */
static void gpio_in_data_clear(struct t_gpio_in_data *data) {
    close_fd(&data->fd);
    close_fd(&data->timer_fd);
    FREE_SDS(data->action_falling);
    FREE_SDS(data->action_rising);
    FREE_SDS(data->long_press_action);
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
 * Frees pointers and closes file descriptors from this node.
 * @param data gpio out data to clear
 */
static void gpio_out_data_clear(struct t_gpio_out_data *data) {
    (void)data;
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node gpio out config node to clear
 */
static void gpio_node_out_clear(struct t_list_node *node) {
    struct t_gpio_out_data *data = (struct t_gpio_out_data *)node->data;
    gpio_out_data_clear(data);
}

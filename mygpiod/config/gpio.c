/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief GPIO configuration
 */

#include "compile_time.h"
#include "mygpiod/config/gpio.h"

#include "mygpio-common/util.h"
#include "mygpiod/actions/actions.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/gpio/util.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/sds_extras.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

// Private definitions
static struct t_action *action_node_data_from_value(sds value);

// Public functions

/**
 * 
 * @param gpios_in Pointer to list of input GPIOs
 * @param gpios_out  Pointer to list of output GPIOs
 * @param gpio_dir GPIO config folder
 * @return true on success, else false
 */
bool gpio_read_dir(struct t_list *gpios_in, struct t_list *gpios_out, sds gpio_dir) {
        errno = 0;
    DIR *dir = opendir(gpio_dir);
    if (dir == NULL) {
        MYGPIOD_LOG_ERROR("Error opening directory \"%s\"", gpio_dir);
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
                    MYGPIOD_LOG_DEBUG("Parsing %s/%s", gpio_dir, next_file->d_name);
                    struct t_gpio_in_data *data = gpio_in_data_new();
                    bool rc = parse_gpio_config_file(GPIOD_LINE_DIRECTION_INPUT, data, gpio_dir, next_file->d_name);
                    if (rc == true && list_push(gpios_in, gpio, data) == true) {
                        i++;
                        continue;
                    }
                    gpio_in_data_clear(data);
                    FREE_PTR(data);
                }
                else if (strcmp(rest, "out") == 0) {
                    MYGPIOD_LOG_DEBUG("Parsing %s/%s", gpio_dir, next_file->d_name);
                    struct t_gpio_out_data *data = gpio_out_data_new();
                    bool rc = parse_gpio_config_file(GPIOD_LINE_DIRECTION_OUTPUT, data, gpio_dir, next_file->d_name);
                    if (rc == true && list_push(gpios_out, gpio, data) == true) {
                        i++;
                        continue;
                    }
                    gpio_out_data_clear(data);
                    FREE_PTR(data);
                }
            }
        }
        MYGPIOD_LOG_WARN("Skipping file %s/%s", gpio_dir, next_file->d_name);
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
 * Parses a gpio configuration file
 * @param direction one off GPIOD_LINE_DIRECTION_INPUT, GPIOD_LINE_DIRECTION_OUTPUT
 * @param data already allocated gpio config data to populate
 * @param dirname directory name
 * @param filename file name
 * @return true on success, else false
 */
bool parse_gpio_config_file(int direction, void *data, const char *dirname, const char *filename) {
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
bool parse_gpio_config_file_out_kv(sds key, sds value, struct t_gpio_out_data *data) {
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
bool parse_gpio_config_file_in_kv(sds key, sds value, struct t_gpio_in_data *data) {
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
 * Creates a new gpio in config data node and sets its values to defaults.
 * @return newly allocated struct t_gpio_node_in
 */
struct t_gpio_in_data *gpio_in_data_new(void) {
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
    data->name = sdsempty();
    return data;
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param data gpio in config node to clear
 */
void gpio_in_data_clear(struct t_gpio_in_data *data) {
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
    sdsfree(data->name);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node gpio in config node to clear
 */
void gpio_node_in_clear(struct t_list_node *node) {
    struct t_gpio_in_data *data = (struct t_gpio_in_data *)node->data;
    gpio_in_data_clear(data);
}

/**
 * Creates a new gpio out config data node and sets its values to defaults.
 * @return newly allocated struct t_gpio_out_data
 */
struct t_gpio_out_data *gpio_out_data_new(void) {
    struct t_gpio_out_data *data = malloc_assert(sizeof(struct t_gpio_out_data));
    data->drive = GPIOD_LINE_DRIVE_PUSH_PULL;
    data->value = GPIOD_LINE_VALUE_INACTIVE;
    data->timer_fd = -1;
    data->request = NULL;
    data->name = sdsempty();
    return data;
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param data gpio out data to clear
 */
void gpio_out_data_clear(struct t_gpio_out_data *data) {
    close_fd(&data->timer_fd);
    if (data->request != NULL) {
        gpiod_line_request_release(data->request);
    }
    sdsfree(data->name);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node gpio out config node to clear
 */
void gpio_node_out_clear(struct t_list_node *node) {
    struct t_gpio_out_data *data = (struct t_gpio_out_data *)node->data;
    gpio_out_data_clear(data);
}

/**
 * Clears the gpio part of the config
 * @param gpios_in Pointer to list of input GPIOs
 * @param gpios_out Pointer to list of output GPIOs
 */
void gpios_config_clear(struct t_list *gpios_in, struct t_list *gpios_out) {
    list_clear(gpios_in, gpio_node_in_clear);
    list_clear(gpios_out, gpio_node_out_clear);
}

// Private functions

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

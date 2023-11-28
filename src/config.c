/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "config.h"

#include "list.h"
#include "log.h"
#include "util.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//private definitions
static bool parse_config_line(unsigned line_num, char *line, struct t_config *config);
static bool parse_gpio_config_file(enum gpio_modes mode, void *node, const char *dirname, const char *filename);
static bool parse_gpio_config_file_in_line(unsigned line_num, char *line, struct t_gpio_node_in *node);
static bool parse_gpio_config_file_out_line(unsigned line_num, char *line, struct t_gpio_node_out *node);
static struct t_gpio_node_in *gpio_node_in_new(void);
static struct t_gpio_node_out *gpio_node_out_new(void);
static void gpio_node_in_clear(void *node);
static void gpio_node_out_clear(void *node);
static char *skip_chars(char *line, size_t skip, char c);
static char *chomp(char *line);

//public functions

/**
 * Allocates a new config struct and sets defaults
 * @return allocated struct with default values assigned.
 */
struct t_config *config_new(void) {
    struct t_config *config = malloc(sizeof(struct t_config));
    if (config == NULL) {
        return NULL;
    }
    config->signal_fd = make_signalfd();
    if (config->signal_fd == -1) {
        free(config);
        return NULL;
    }
    list_init(&config->gpios_in);
    list_init(&config->gpios_out);
    config->chip = strdup(CFG_CHIP);
    config->active_low = CFG_ACTIVE_LOW;
    config->bias = CFG_BIAS;
    config->event_request = CFG_EVENT_CHIP;
    config->loglevel = loglevel;
    config->syslog = CFG_SYSLOG;
    config->dir_gpio = strdup(CFG_GPIO_DIR);
    return config;
}

/**
 * Clears the config struct.
 * @param config pointer to config to free
 */
void config_clear(struct t_config *config) {
    list_clear(&config->gpios_in, gpio_node_in_clear);
    list_clear(&config->gpios_out, gpio_node_out_clear);
    if (config->signal_fd > 0) {
        close(config->signal_fd);
    }
    free(config->chip);
    free(config->dir_gpio);
}

/**
 * Reads the mygpiod config file
 * @param config config struct to populate
 * @param config_file config filename to read
 * @return true on success, else false
 */
bool config_read(struct t_config *config, const char *config_file) {
    FILE *fp = fopen(config_file, "re");
    if (fp == NULL) {
        MYGPIOD_LOG_ERROR("Can not open \"%s\": %s", config_file, strerror(errno));
        return false;
    }
    
    char *line = NULL;
    size_t n = 0;
    bool rc = true;
    unsigned line_num = 0;
    while (getline(&line, &n, fp) > 0) {
        line_num++;
        //strip whitespace characters
        char *line_c = chomp(line);
        if (line_c == NULL) {
            continue;
        }
        line_c = skip_chars(line_c, 0, ' ');
        
        //ignore blank lines and comments
        if (line_c[0] == '#' ||
            line_c[0] == '\0')
        {
            continue;
        }
        if (parse_config_line(line_num, line_c, config) == false) {
            rc = false;
            break;
        }
    }
    if (line != NULL) {
        free(line);
    }
    fclose(fp);

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
                    struct t_gpio_node_in *node = gpio_node_in_new();
                    rc = parse_gpio_config_file(GPIO_MODE_INPUT, node, config->dir_gpio, next_file->d_name);
                    if (rc == true && list_push(&config->gpios_in, gpio, node) == true) {
                        i++;
                        continue;
                    }
                    gpio_node_in_clear(node);
                    free(node);
                }
                else if (strcmp(rest, "out") == 0) {
                    struct t_gpio_node_out *node = gpio_node_out_new();
                    rc = parse_gpio_config_file(GPIO_MODE_OUTPUT, node, config->dir_gpio, next_file->d_name);
                    if (rc == true && list_push(&config->gpios_out, gpio, node) == true) {
                        i++;
                        continue;
                    }
                    gpio_node_in_clear(node);
                    free(node);
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
    return rc;
}

//private functions

/**
 * Parses a configuration line
 * @param line line to parse
 * @param config already allocated config struct to populate
 * @return true on success, else false
 */
static bool parse_config_line(unsigned line_num, char *line, struct t_config *config) {
    if (strncmp(line, "chip", 4) == 0) {
        line = skip_chars(line, 4, '=');
        free(config->chip);
        config->chip = strdup(line);
        MYGPIOD_LOG_INFO("Setting chip to \"%s\"", config->chip);
        return true;
    }
    if (strncmp(line, "event", 5) == 0) {
        line = skip_chars(line, 5, '=');
        config->event_request = parse_event_request(line);
        MYGPIOD_LOG_INFO("Setting event to \"%s\"", lookup_event_request(config->event_request));
        return true;
    }
    if (strncmp(line, "active_low", 10) == 0) {
        line = skip_chars(line, 10, '=');
        config->active_low = parse_bool(line);
        MYGPIOD_LOG_INFO("Setting active_low to \"%s\"", bool_to_str(config->active_low));
        return true;
    }
    if (strncmp(line, "bias", 4) == 0) {
        line = skip_chars(line, 4, '=');
        config->bias = parse_bias(line);
        MYGPIOD_LOG_INFO("Setting bias to \"%s\"", lookup_bias(config->bias));
        return true;
    }
    if (strncmp(line, "loglevel", 8) == 0) {
        line = skip_chars(line, 8, '=');
        config->loglevel = parse_loglevel(line);
        MYGPIOD_LOG_INFO("Setting loglevel to \"%s\"", lookup_loglevel(config->loglevel));
        return true;
    }
    if (strncmp(line, "syslog", 6) == 0) {
        line = skip_chars(line, 6, '=');
        config->syslog = parse_bool(line);
        MYGPIOD_LOG_INFO("Setting syslog to \"%s\"", bool_to_str(config->syslog));
        return true;
    }
    if (strncmp(line, "gpio_dir", 8) == 0) {
        line = skip_chars(line, 8, '=');
        free(config->dir_gpio);
        config->dir_gpio = strdup(line);
        MYGPIOD_LOG_INFO("Setting gpio_dir to \"%s\"", config->dir_gpio);
        return true;
    }

    //invalid
    MYGPIOD_LOG_ERROR("Invalid configuration line #%u", line_num);
    return false;
}

static bool parse_gpio_config_file(enum gpio_modes mode, void *node, const char *dirname, const char *filename) {
    size_t filepath_len = strlen(dirname) + 1 + strlen(filename) + 1;
    char *filepath = malloc(filepath_len);
    snprintf(filepath, filepath_len, "%s/%s", dirname, filename);
    FILE *fp = fopen(filepath, "ro");
    if (fp == NULL) {
        MYGPIOD_LOG_ERROR("Error opening %s", filepath);
        free(filepath);
        return false;
    }
    free(filepath);
    char *line = NULL;
    size_t n = 0;
    bool rc = true;
    unsigned line_num = 0;
    while (getline(&line, &n, fp) > 0) {
        line_num++;
        //strip whitespace characters
        char *line_c = chomp(line);
        if (line_c == NULL) {
            continue;
        }
        line_c = skip_chars(line_c, 0, ' ');
        
        //ignore blank lines and comments
        if (line_c[0] == '#' ||
            line_c[0] == '\0')
        {
            continue;
        }
        if (mode == GPIO_MODE_OUTPUT) {
            if (parse_gpio_config_file_out_line(line_num, line_c, node) == false) {
                rc = false;
                break;
            }
            continue;
        }

        if (parse_gpio_config_file_in_line(line_num, line_c, node) == false) {
            rc = false;
            break;
        }
    }
    if (line != NULL) {
        free(line);
    }
    fclose(fp);
    return rc;
}

static bool parse_gpio_config_file_out_line(unsigned line_num, char *line, struct t_gpio_node_out *node) {
    if (strncmp(line, "value", 5) == 0) {
        line = skip_chars(line, 5, '=');
        if (parse_int(line, &node->value, NULL, 0, 1) == true) {
            return true;
        }
    }

    //invalid
    MYGPIOD_LOG_ERROR("Invalid configuration line #%u", line_num);
    return false;
}

static bool parse_gpio_config_file_in_line(unsigned line_num, char *line, struct t_gpio_node_in *node) {
    if (strncmp(line, "event", 5) == 0) {
        line = skip_chars(line, 5, '=');
        node->event = parse_event(line);
        return true;
    }
    if (strncmp(line, "cmd", 3) == 0) {
        line = skip_chars(line, 3, '=');
        free(node->cmd);
        node->cmd = strdup(line);
        return true;
    }
    if (strncmp(line, "long_press_timeout", 18) == 0) {
        line = skip_chars(line, 18, '=');
        if (parse_int(line, &node->long_press_timeout, NULL, 0, 9) == true) {
            return true;
        }
    }
    if (strncmp(line, "long_press_event", 16) == 0) {
        line = skip_chars(line, 16, '=');
        node->long_press_event = parse_event(line);
        return true;
    }
    if (strncmp(line, "long_press_cmd", 14) == 0) {
        line = skip_chars(line, 14, '=');
        free(node->long_press_cmd);
        node->long_press_cmd = strdup(line);
        return true;
    }

    //invalid
    MYGPIOD_LOG_ERROR("Invalid configuration line #%u", line_num);
    return false;
}

/**
 * Creates a new config node and sets its values to defaults
 * @param gpio gpio number
 * @return newly allocated struct t_gpio_node
 */
static struct t_gpio_node_in *gpio_node_in_new(void) {
    struct t_gpio_node_in *node = malloc(sizeof(struct t_gpio_node_in));
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Out of memory");
        return NULL;
    }
    node->cmd = NULL;
    node->long_press_timeout = 0;
    node->long_press_cmd = NULL;
    node->long_press_event = 0;
    node->ignore_event = false;
    node->timer_fd = -1;
    node->fd = -1;
    return node;
}

static struct t_gpio_node_out *gpio_node_out_new(void) {
    struct t_gpio_node_out *node = malloc(sizeof(struct t_gpio_node_out));
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Out of memory");
        return NULL;
    }
    node->value = 0;
    return node;
}

static void gpio_node_in_clear(void *node) {
    struct t_gpio_node_in *data = (struct t_gpio_node_in *)node;
    if (data->fd > -1) {
        close(data->fd);
    }
    if (data->timer_fd > -1) {
        close(data->timer_fd);
    }
    if (data->cmd != NULL) {
        free(data->cmd);
    }
    if (data->long_press_cmd != NULL) {
        free(data->long_press_cmd);
    }
}

static void gpio_node_out_clear(void *node) {
    (void)node;
    return;
}

/**
 * Skips chars and following whitespaces and defined char c
 * @param line string
 * @param gpio number of chars to skip
 * @param c char to skip
 * @return pointer to new position in string
 */
static char *skip_chars(char *line, size_t skip, char c) {
    if (strlen(line) < skip) {
        //skip should not be greater than string length
        skip = strlen(line);
    }
    line += skip;
    while ((isspace(line[0]) || line[0] == c) && line[0] != '\0') {
        line++;
    }
    return line;
}

/**
 * Removes whitespace characters from end
 * @param line string to chomp
 * @return chomped string
 */
static char *chomp(char *line) {
    size_t i = strlen(line) - 1;
    while (i > 0 && isspace(line[i])) {
        i--;
    }
    line[i + 1] = '\0';
    return line;
}

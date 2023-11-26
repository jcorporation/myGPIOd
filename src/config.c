/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "config.h"

#include "log.h"
#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//private definitions
static bool parse_config_line(char *line, struct t_config *config);
static bool parse_gpio_input_line(struct t_gpio_node *node, char *line);
static bool parse_gpio_output_line(struct t_gpio_node *node, char *line);
static bool config_node_push(struct t_config *config, struct t_gpio_node *node);
static struct t_gpio_node *config_node_new(unsigned gpio);
static void config_node_clear(struct t_gpio_node *node);
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
    config->gpios = NULL;
    config->gpios_tail = NULL;
    config->length = 0;
    config->chip = strdup(CFG_CHIP);
    config->active_low = CFG_ACTIVE_LOW;
    config->bias = CFG_BIAS;
    config->edge = CFG_EDGE;
    config->loglevel = loglevel;
    config->syslog = CFG_SYSLOG;
    config->startup_time = time(NULL);
    config->delayed_event.timer_fd = -1;
    config->delayed_event.cn = NULL;
    return config;
}

/**
 * Clears the config struct.
 * @param config pointer to config to free
 */
void config_clear(struct t_config *config) {
    struct t_gpio_node *current = config->gpios;
    struct t_gpio_node *tmp = NULL;
    while (current != NULL) {
        config_node_clear(current);
        tmp = current;
        current = current->next;
        free(tmp);
    }
    if (config->chip != NULL) {
        free(config->chip);
    }
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
    while (getline(&line, &n, fp) > 0) {
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
        if (parse_config_line(line_c, config) == false) {
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

//private functions

/**
 * Parses a configuration line
 * @param line line to parse
 * @param config already allocated config struct to populate
 * @return true on success, else false
 */
static bool parse_config_line(char *line, struct t_config *config) {
    //global values
    if (strncmp(line, "chip", 4) == 0) {
        line = skip_chars(line, 4, '=');
        if (parse_chip(line, config) == false) {
            MYGPIOD_LOG_ERROR("Invalid chip number");
            return false;
        }
        MYGPIOD_LOG_INFO("Setting chip to \"%s\"", config->chip);
        return true;
    }
    if (strncmp(line, "edge", 4) == 0) {
        line = skip_chars(line, 4, '=');
        config->edge = parse_edge(line);
        MYGPIOD_LOG_INFO("Setting edge to \"%s\"", lookup_ctxless_event(config->edge));
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

    //gpio lines
    char *rest = NULL;
    unsigned gpio;
    if (parse_uint(line, &gpio, &rest, 1, 99) == false) {
        MYGPIOD_LOG_ERROR("First value is not a valid gpio number");
        return false;
    }
    MYGPIOD_LOG_DEBUG("Processing configuration line for gpio %u", gpio);
    //line seems to be valid, create a struct for config_node
    struct t_gpio_node *node = config_node_new(gpio);
    if (node == NULL) {
        return false;
    }
    line = rest;
    line = skip_chars(line, 0, ',');
    if (strncmp(line, "in", 2) == 0) {
        if (parse_gpio_input_line(node, line) == true &&
            config_node_push(config, node) == true)
        {
            MYGPIOD_LOG_INFO("Added gpio: \"%u\", mode: \"input\", edge: \"%s\", long_press: \"%d\", cmd: \"%s\"",
                node->gpio, lookup_ctxless_event(node->edge), node->long_press, node->cmd);
            return true;
        }
    }
    else if (strncmp(line, "out", 3) == 0) {
        if (parse_gpio_output_line(node, line) == true &&
            config_node_push(config, node) == true)
        {
            MYGPIOD_LOG_INFO("Added gpio: \"%u\", mode: \"output\", value: \"%s\"",
                node->gpio, lookup_gpio_value(node->value));
            return true;
        }
    }

    //invalid
    MYGPIOD_LOG_ERROR("Invalid configuration line");
    config_node_clear(node);
    free(node);
    return false;
}

/**
 * Parses a gpio config input line
 * @param node already allocated config node to populate
 * @param line line to parse
 * @return true on success, else false
 */
static bool parse_gpio_input_line(struct t_gpio_node *node, char *line) {
    line = skip_chars(line, 2, ',');
    char edge[8] = { 0 };
    for (int i = 0; i < 8 && line[0] != ','; i++, line++) {
        if (line[0] == '\0') {
            return false;
        }
        edge[i] = line[0];
    }
    node->edge = parse_edge(edge);

    //goto next value
    line = skip_chars(line, 0, ',');

    //long press timeout
    char *rest;
    if (parse_int(line, &node->long_press, &rest, 0, 99) == false) {
        MYGPIOD_LOG_ERROR("Long press value is invalid");
        return false;
    }
    line = rest;

    //goto next value
    line = skip_chars(line, 0, ',');

    //last value is cmd
    if (line[0] != '\0') {
        node->cmd = strdup(line);
    }
    return true;
}

/**
 * Parses a gpio config output line
 * @param node already allocated config node to populate
 * @param line line to parse
 * @return true on success, else false
 */
static bool parse_gpio_output_line(struct t_gpio_node *node, char *line) {
    line = skip_chars(line, 3, ',');
    if (strcasecmp(line, "high") == 0) {
        node->value = GPIO_VALUE_HIGH;
        return true;
    }
    if (strcasecmp(line, "low") == 0) {
        node->value = GPIO_VALUE_LOW;
        return true;
    }
    return false;
}

/**
 * Appends a config node
 * @param config pointer to config struct
 * @param node config line to append
 * @return true on success, else false
 */
static bool config_node_push(struct t_config *config, struct t_gpio_node *node) {
    node->next = NULL;

    if (config->gpios == NULL) {
        config->gpios = node;
    }
    else if (config->gpios_tail != NULL) {
        config->gpios_tail->next = node;
    }
    else {
        return false;
    }

    config->gpios_tail = node;
    config->length++;
    return true;
}

/**
 * Creates a new config node and sets its values to defaults
 * @param gpio gpio number
 * @return newly allocated struct t_gpio_node
 */
static struct t_gpio_node *config_node_new(unsigned gpio) {
    struct t_gpio_node *node = malloc(sizeof(struct t_gpio_node));
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Out of memory");
        return NULL;
    }
    node->gpio = gpio;
    node->cmd = NULL;
    node->long_press = 0;
    node->ignore_event = false;
    node->value = GPIO_VALUE_UNSET;
    return node;
}

/**
 * Clears the config node
 * @param cl node to clear
 */
static void config_node_clear(struct t_gpio_node *node) {
    if (node->cmd != NULL) {
        free(node->cmd);
    }
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

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "config.h"
#include "log.h"

#include <ctype.h>
#include <errno.h>
#include <gpiod.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//private definitions
static bool config_node_push(struct t_config *config, struct t_config_node *node);
static void config_node_clear(struct t_config_node *node);
static char *skip_chars(char *line, size_t offset, char c);
static char *chomp(char *line);

//public functions

/**
 * Parses the bias flags
 * @param option string to parse
 * @return parsed bias flag
 */
int bias_flags(const char *option) {
    if (strcmp(option, "pull-down") == 0) {
        return GPIOD_CTXLESS_FLAG_BIAS_PULL_DOWN;
    }
    if (strcmp(option, "pull-up") == 0) {
        return GPIOD_CTXLESS_FLAG_BIAS_PULL_UP;
    }
    if (strcmp(option, "disable") == 0) {
        return GPIOD_CTXLESS_FLAG_BIAS_DISABLE;
    }
    // Bias set to as-is
    return 0;
}

/**
 * Allocates a new config struct and sets defaults
 * @return allocated struct with default values assigned.
 */
struct t_config *config_new(void) {
    struct t_config *config = malloc(sizeof(struct t_config));
    if (config == NULL) {
        return NULL;
    }
    config->head = NULL;
    config->tail = NULL;
    config->length = 0;
    config->chip = strdup("0");
    config->active_low = true;
    config->bias = strdup("as-is");
    config->edge = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
    config->loglevel = loglevel;
    config->startup_time = time(NULL);
    config->syslog = false;
    return config;
}

/**
 * Clears the config struct.
 * @param config pointer to config to free
 */
void config_clear(struct t_config *config) {
    struct t_config_node *current = config->head;
    struct t_config_node *tmp = NULL;
    while (current != NULL) {
        config_node_clear(current);
        tmp = current;
        current = current->next;
        free(tmp);
    }
    if (config->chip != NULL) {
        free(config->chip);
        config->chip = NULL;
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
    unsigned i = 0;
    while (getline(&line, &n, fp) > 0) {
        i++;
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

        //global values
        if (strncmp(line_c, "chip", 4) == 0) {
            line_c = skip_chars(line_c, 4, '=');
            if (line_c[0] != '\0') {
                free(config->chip);
                config->chip = strdup(line_c);
                MYGPIOD_LOG_INFO("Setting chip to \"%s\"", line_c);
            }
            else {
                MYGPIOD_LOG_WARN("Empty chip value in line %u", i);
            }
            continue;
        }
        if (strncmp(line_c, "edge", 4) == 0) {
            line_c = skip_chars(line_c, 4, '=');
            if (strcmp(line_c, "falling") == 0) {
                config->edge = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
            }
            else if (strcmp(line_c, "rising") == 0) {
                config->edge = GPIOD_CTXLESS_EVENT_RISING_EDGE;
            }
            else if (strcmp(line_c, "both") == 0) {
                config->edge = GPIOD_CTXLESS_EVENT_BOTH_EDGES;
            }
            else {
                MYGPIOD_LOG_WARN("Invalid edge value \"%s\" in line %u", line_c, i);
                continue;
            }
            MYGPIOD_LOG_INFO("Setting edge to \"%s\"", line_c);
            continue;
        }
        if (strncmp(line_c, "active_low", 10) == 0) {
            line_c = skip_chars(line_c, 10, '=');
            if (strcmp(line_c, "true") == 0) {
                config->active_low = true;
            }
            else if (strcmp(line_c, "false") == 0) {
                config->active_low = false;
            }
            else {
                MYGPIOD_LOG_WARN("Invalid active_low value \"%s\" in line %u", line_c, i);
                continue;
            }
            MYGPIOD_LOG_INFO("Setting active_low to \"%s\"", line_c);
            continue;
        }
        if (strncmp(line_c, "bias", 4) == 0) {
            line_c = skip_chars(line_c, 4, '=');
            if (strcmp(line_c, "as-is") == 0 ||
                strcmp(line_c, "disable") == 0 ||
                strcmp(line_c, "pull-down") == 0 ||
                strcmp(line_c, "pull-up") == 0)
            {
                free(config->bias);
                config->bias = strdup(line_c);
            }
            else {
                MYGPIOD_LOG_WARN("Invalid bias value in line %u", i);
            }
            continue;
        }
        if (strncmp(line_c, "loglevel", 8) == 0) {
            line_c = skip_chars(line_c, 8, '=');
            config->loglevel = (int)strtoimax(line_c, NULL, 10);
            continue;
        }
        if (strncmp(line_c, "syslog", 6) == 0) {
            line_c = skip_chars(line_c, 6, '=');
            if (strcmp(line_c, "true") == 0) {
                config->syslog = true;
            }
            else if (strcmp(line_c, "false") == 0) {
                config->syslog = false;
            }
            else {
                MYGPIOD_LOG_WARN("Invalid syslog value \"%s\" in line %u", line_c, i);
                continue;
            }
            MYGPIOD_LOG_INFO("Setting syslog to \"%s\"", line_c);
            continue;
        }

        //gpio lines
        char *rest = NULL;
        unsigned gpio = (unsigned)strtoumax(line_c, &rest, 10);
        if (gpio == 0 || errno == ERANGE) {
            MYGPIOD_LOG_WARN("First value is not a valid gpio number in line %u", i);
            continue;
        }
        MYGPIOD_LOG_DEBUG("Processing configuration line for gpio %u", gpio);
        //line seems to be valid, create a struct for config_node
        struct t_config_node *node = (struct t_config_node *) malloc(sizeof(struct t_config_node));
        node->gpio = gpio;
        node->cmd = NULL;
        node->last_execution = 0;

        //goto next value
        line_c = rest;
        line_c = skip_chars(line_c, 0, ',');

        //edge value
        if (strncmp(line_c, "falling", 7) == 0) {
            node->edge = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
            line_c += 7;
        }
        else if (strncmp(line_c, "rising", 6) == 0) {
            node->edge = GPIOD_CTXLESS_EVENT_RISING_EDGE;
            line_c += 6;
        }
        else if (strncmp(line_c, "both", 4) == 0) {
            node->edge = GPIOD_CTXLESS_EVENT_BOTH_EDGES;
            line_c += 4;
        }
        else {
            MYGPIOD_LOG_WARN("Unknown edge value \"%s\" in line %u", line_c, i);
            config_node_clear(node);
            free(node);
            continue;
        }

        //goto next value
        line_c = skip_chars(line_c, 0, ',');

        //last value is cmd
        if (line_c[0] == '\0') {
            MYGPIOD_LOG_WARN("No cmd specified in line %u", i);
            config_node_clear(node);
            free(node);
            continue;
        }
        node->cmd = strdup(line_c);

        if (config_node_push(config, node) == false) {
            MYGPIOD_LOG_WARN("Error adding configuration line %u", i);
            config_node_clear(node);
            free(node);
            continue;
        }
        MYGPIOD_LOG_INFO("Added gpio: \"%u\", edge: \"%d\", cmd: \"%s\"", node->gpio, node->edge, node->cmd);
    }
    if (line != NULL) {
        free(line);
    }
    fclose(fp);
    return true;
}

//private functions

/**
 * Appends a config node
 * @param config pointer to config struct
 * @param node config line to append
 * @return true on success, else false
 */
static bool config_node_push(struct t_config *config, struct t_config_node *node) {
    node->next = NULL;

    if (config->head == NULL) {
        config->head = node;
    }
    else if (config->tail != NULL) {
        config->tail->next = node;
    }
    else {
        return false;
    }

    config->tail = node;
    config->length++;
    return true;
}

/**
 * Clears the config node
 * @param cl node to clear
 */
static void config_node_clear(struct t_config_node *node) {
    if (node->cmd != NULL) {
        free(node->cmd);
    }
}

/**
 * Skips chars and following whitespaces and defined char c
 * @param line string
 * @param offset number of chars to skip
 * @param c char to skip
 * @return pointer to new position in string
 */
static char *skip_chars(char *line, size_t offset, char c) {
    if (strlen(line) < offset) {
        //offset should not be greater than string length
        offset = strlen(line);
    }
    line += offset;
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

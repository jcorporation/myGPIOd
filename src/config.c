/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myGPIOd (c)2020 Juergen Mang <mail@jcgames.de>
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
static bool config_line_push(struct t_config *c, struct t_config_line *cl);
static void config_line_free(struct t_config_line *cl);
static char *skip_chars(char *line, unsigned offset, char c);

//private functions
static bool config_line_push(struct t_config *c, struct t_config_line *cl) {
    cl->next = NULL;

    if (c->head == NULL) {
        c->head = cl;
    }
    else if (c->tail != NULL) {
        c->tail->next = cl;
    }
    else {
        return false;
    }

    c->tail = cl;
    c->length++;
    return true;
}

static void config_line_free(struct t_config_line *cl) {
    if (cl->cmd != NULL) {
        free(cl->cmd);
    }
}

static char *skip_chars(char *line, unsigned offset, char c) {
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

static char *chomp(char *line) {
    size_t i = strlen(line) - 1;
    while (i > 0 && isspace(line[i])) {
        i--;
    }
    line[i + 1] = '\0';
    return line;
}

//public functions
bool config_free(struct t_config *c) {
    struct t_config_line *current = c->head;
    struct t_config_line *tmp = NULL;
    while (current != NULL) {
        config_line_free(current);
        tmp = current;
        current = current->next;
        free(tmp);
    }
    if (c->chip != NULL) {
        free(c->chip);
        c->chip = NULL;
    }
    return true;
}

bool read_config(struct t_config *config, const char *config_file) {
    FILE *fp = fopen(config_file, "re");
    if (fp == NULL) {
        LOG_ERROR("Can not open \"%s\": %s", config_file, strerror(errno));
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
        if (line_c[0] == '#' || line_c[0] == '\0') {
            continue;
        }
        
        //global values
        if (strncmp(line_c, "chip", 4) == 0) {
            line_c = skip_chars(line_c, 4, '=');
            if (line_c[0] != '\0') {
                free(config->chip);
                config->chip = strdup(line_c);
                LOG_VERBOSE("Setting chip to \"%s\"", line_c);
            }
            else {
                LOG_WARN("Empty chip value in line %u", i);
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
                LOG_WARN("Invalid edge value \"%s\" in line %u", line_c, i);
                continue;
            }
            LOG_VERBOSE("Setting edge to \"%s\"", line_c);
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
                LOG_WARN("Invalid active_low value \"%s\" in line %u", line_c, i);
                continue;
            }
            LOG_VERBOSE("Setting active_low to \"%s\"", line_c);
            continue;
        }
        if (strncmp(line_c, "loglevel", 8) == 0) {
            line_c = skip_chars(line_c, 8, '=');
            config->loglevel = strtoimax(line_c, NULL, 10);
            #ifdef DEBUG
            LOG_DEBUG("Ignoring loglevel in debug mode");
            #else
            set_loglevel(config->loglevel);
            #endif
            continue;
        }
        
        //gpio lines
        char *rest = NULL;
        unsigned gpio = strtoumax(line_c, &rest, 10);
        if (gpio == 0 || errno == ERANGE) {
            LOG_WARN("First value is not a valid gpio number in line %u", i);
            continue;
        }
        LOG_DEBUG("Processing configuration line for gpio %u", gpio);
        //line seems to be valid, create a struct for config_line
        struct t_config_line *cl = (struct t_config_line *) malloc(sizeof(struct t_config_line));
        cl->gpio = gpio;
        cl->cmd = NULL;
        cl->last_execution = 0;
        
        //goto next value
        line_c = rest;
        line_c = skip_chars(line_c, 0, ',');

        //edge value
        if (strncmp(line_c, "falling", 7) == 0) {
            cl->edge = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
            line_c += 7;
        }
        else if (strncmp(line_c, "rising", 6) == 0) {
            cl->edge = GPIOD_CTXLESS_EVENT_RISING_EDGE;
            line_c += 6;
        }
        else if (strncmp(line_c, "both", 4) == 0) {
            cl->edge = GPIOD_CTXLESS_EVENT_BOTH_EDGES;
            line_c += 4;
        }
        else {
            LOG_WARN("Unknown edge value \"%s\" in line %u", line_c, i);
            config_line_free(cl);
            free(cl);
            continue;
        }
        
        //goto next value
        line_c = skip_chars(line_c, 0, ',');

        //last value is cmd
        if (line_c[0] == '\0') {
            LOG_WARN("No cmd specified in line %u", i);
            config_line_free(cl);
            free(cl);
            continue;
        }
        cl->cmd = strdup(line_c);

        if (config_line_push(config, cl) == false) {
            LOG_WARN("Error adding configuration line %u", i);
            config_line_free(cl);
            free(cl);
            continue;
        }
        LOG_VERBOSE("Added gpio: \"%u\", edge: \"%d\", cmd: \"%s\"", cl->gpio, cl->edge, cl->cmd);
    }
    if (line != NULL) {
        free(line);
    }
    fclose(fp);
    return true;
}

/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myGPIOd (c)2020 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <gpiod.h>

#include "log.h"
#include "config.h"

bool config_line_push(struct t_config *c, struct t_config_line *cl) {
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

void config_line_free(struct t_config_line *cl) {
    if (cl->cmd != NULL) {
        free(cl->cmd);
    }
}

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
        
    }
    return true;
}

bool read_config(struct t_config *config, const char *config_file) {
    FILE *fp = fopen(config_file, "r");
    if (fp == NULL) {
        LOG_ERROR("Can not open %s: %s", config_file, strerror(errno));
        return false;
    }
    
    char *line = NULL;
    size_t n = 0;
    ssize_t read;
    unsigned i = 0;
    while ((read = getline(&line, &n, fp)) > 0) {
        i++;
        if (read > 0) {
            if (line[0] == '#' || line[0] == '\n' || 
                line[0] == ' ' || line[0] == '\t')
            {
                continue;
            }
        }
        else {
            continue;
        }
        //remove newline
        char *line_c = strtok(line, "\n");
        
        //global values
        if (strstr(line_c, "chip=") != NULL) {
            char *chip = strstr(line_c, "=");
            if (strlen(chip) > 0) {
                chip++;
                free(config->chip);
                config->chip = strdup(chip);
            }
            continue;
        }
        else if (strstr(line_c, "edge=") != NULL) {
            if (strstr(line, "falling") != NULL) {
                config->edge = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
            }
            else if (strstr(line, "rising") != NULL) {
                config->edge = GPIOD_CTXLESS_EVENT_RISING_EDGE;
            }
            else if (strstr(line, "both") != NULL) {
                config->edge = GPIOD_CTXLESS_EVENT_BOTH_EDGES;
            }
            continue;
        }
        else if (strstr(line_c, "active_low=") != NULL) {
            if (strstr(line, "true") != NULL) {
                config->active_low = true;
            }
            else {
                config->active_low = false;
            }
            continue;
        }
        
        //gpio lines
        struct t_config_line *cl = (struct t_config_line *) malloc(sizeof(struct t_config_line));
        cl->cmd = NULL;
        cl->last_execution = 0;

        char *token = strtok(line_c, ",");
        if (token == NULL) {
            free(cl);
            continue;
        }
        cl->gpio = strtoumax(token, NULL, 10);

        if ((token = strtok(NULL, ",")) == NULL) {
            LOG_WARN("Incomplete configuration line %u", i);
            config_line_free(cl);
            free(cl);
            continue;
        }
        if (strcmp(token, "falling") == 0) {
            cl->edge = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
        }
        else if (strcmp(token, "rising") == 0) {
            cl->edge = GPIOD_CTXLESS_EVENT_RISING_EDGE;
        }
        else if (strcmp(token, "both") == 0) {
            cl->edge = GPIOD_CTXLESS_EVENT_BOTH_EDGES;
        }
        else {
            LOG_WARN("Unknown edge value in line %u", i);
            config_line_free(cl);
            free(cl);
            continue;
        }

        if ((token = strtok(NULL, ",")) == NULL) {
            LOG_WARN("Incomplete configuration line %u", i);
            config_line_free(cl);
            free(cl);
            continue;
        }
        cl->cmd = strdup(token);

        if (config_line_push(config, cl) == false) {
            LOG_WARN("Error adding configuration line %u", i);
            config_line_free(cl);
            free(cl);
            continue;
        }
    }
    if (line != NULL) {
        free(line);
    }
    fclose(fp);
    return true;
}

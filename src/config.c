#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "config.h"

struct t_config_line *get_config_from_fd(struct t_config *c, int fd) {
    struct t_config_line *current = c->head;
 
    while (current != NULL) {
        if (current->fd == fd) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}


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
    return true;
}

void config_line_free(struct t_config_line *cl) {
    if (cl->direction != NULL) {
        free(cl->direction);
    }
    if (cl->edge != NULL) {
        free(cl->edge);
    }
    if (cl->active_low != NULL) {
        free(cl->active_low);
    }
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
    return true;
}

bool read_config(struct t_config *config) {
    FILE *fp = fopen("/etc/mygpiod.conf", "r");
    if (fp == false) {
        fprintf(stderr, "Can not open /etc/mygpiod.conf");
        return false;
    }
    
    char *line = NULL;
    size_t n = 0;
    ssize_t read;
    while ((read = getline(&line, &n, fp)) > 0) {
        char *crap;
        struct t_config_line *cl = (struct t_config_line *) malloc(sizeof(struct t_config_line));
        cl->direction = NULL;
        cl->edge = NULL;
        cl->active_low = NULL;
        cl->cmd = NULL;
        
        char *line_c = strtok(line, "\n");
        
        char *token = strtok(line_c, ",");
        if (token == NULL) {
            free(cl);
            continue;
        }
        cl->gpio = strtoumax(token, &crap, 10);
        
        if ((token = strtok(NULL, ",")) == NULL) {
            free(cl);
            continue;
        }
        cl->direction = strdup(token);

        if ((token = strtok(NULL, ",")) == NULL) {
            config_line_free(cl);
            free(cl);
            continue;
        }
        cl->edge = strdup(token);
        
        if ((token = strtok(NULL, ",")) == NULL) {
            config_line_free(cl);
            free(cl);
            continue;
        }
        cl->active_low = strdup(token);

        if ((token = strtok(NULL, ",")) == NULL) {
            config_line_free(cl);
            free(cl);
            continue;
        }
        cl->cmd = strdup(token);

        if (config_line_push(config, cl) == false) {
            config_line_free(cl);
            free(cl);
            continue;
        }
    }
    fclose(fp);
    return true;
}

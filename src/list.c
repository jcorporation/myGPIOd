/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "list.h"

#include <stddef.h>
#include <stdlib.h>

void list_init(struct t_list *list) {
    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
}

void list_clear(struct t_list *list, list_data_clear clear_data_callback) {
    struct t_list_node *current = list->head;
    struct t_list_node *tmp;
    while (current != NULL) {
        tmp = current;
        current = current->next;
        clear_data_callback(tmp->data);
        free(tmp->data);
        free(tmp);
    }
    list_init(list);
}

bool list_push(struct t_list *list, unsigned gpio, void *data) {
    //create new node
    struct t_list_node *node = malloc(sizeof(struct t_list_node));
    node->gpio = gpio;
    node->data = data;
    node->next = NULL;

    if (list->head == NULL) {
        //first entry in the list
        list->head = node;
    }
    else {
        //append to the list
        list->tail->next = node;
    }
    //set tail and increase length
    list->tail = node;
    list->length++;
    return true;
}

struct t_list_node *list_node_at(struct t_list *list, unsigned pos) {
    //if there's no data in the list, fail
    if (list->head == NULL ||
        pos >= list->length)
    {
        return NULL;
    }
    struct t_list_node *current = list->head;
    for (; pos > 0; pos--) {
        current = current->next;
    }
    return current;
}

struct t_list_node *list_node_by_gpio(struct t_list *list, unsigned gpio) {
    struct t_list_node *current = list->head;
    while (current != NULL) {
        if (current->gpio == gpio) {
            return current;
        }
    }
    return NULL;
}

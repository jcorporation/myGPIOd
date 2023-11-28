/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/**
 * This file implements some functions for a simply linked list.
 * This list is designed to hold gpio config data.
 */

#include "compile_time.h"
#include "list.h"

#include <stddef.h>
#include <stdlib.h>

/**
 * Initializes the list
 * @param list pointer to already allocated list to initialize
 */
void list_init(struct t_list *list) {
    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
}

/**
 * Frees all list nodes and re-initializes the list.
 * @param list list to clear
 * @param clear_data_callback callback function to clear the nodes data pointer
 */
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

/**
 * Creates a new node and appends it to the list
 * @param list list to append the new node
 * @param gpio the gpio number
 * @param data already allocated data pointer for the new list node
 * @return true on success, else false
 */
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

/**
 * Gets a list node at specified index
 * @param list pointer to the list
 * @param idx index to get
 * @return found list node or NULL on error
 */
struct t_list_node *list_node_at(struct t_list *list, unsigned idx) {
    //if there's no data in the list, fail
    if (list->head == NULL ||
        idx >= list->length)
    {
        return NULL;
    }
    struct t_list_node *current = list->head;
    for (; idx > 0; idx--) {
        current = current->next;
    }
    return current;
}

/**
 * Gets a list node by its gpio number
 * @param list pointer to the list
 * @param gpio gpio number
 * @return found list node or NULL on error
 */
struct t_list_node *list_node_by_gpio(struct t_list *list, unsigned gpio) {
    struct t_list_node *current = list->head;
    while (current != NULL) {
        if (current->gpio == gpio) {
            return current;
        }
    }
    return NULL;
}

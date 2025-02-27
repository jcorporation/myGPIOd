/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_LIST_H
#define MYGPIOD_LIST_H

#include <stdbool.h>

/**
 * A linked list node
 */
struct t_list_node {
    unsigned id;               //!< node id
    void *data;                //!< data pointer
    struct t_list_node *next;  //!< pointer to next node in the list
};

/**
 * Holds the linked list data
 */
struct t_list {
    struct t_list_node *head;  //!< pointer to the head of the list
    struct t_list_node *tail;  //!< pointer to the tail of the list
    unsigned length;           //!< list length
};

typedef void (*list_data_clear)(struct t_list_node *node);

void list_init(struct t_list *list);
void list_clear(struct t_list *list, list_data_clear clear_data_callback);
void list_node_free(struct t_list_node *node, list_data_clear clear_data_callback);
bool list_push(struct t_list *list, unsigned id, void *data);
struct t_list_node *list_node_by_id(struct t_list *list, unsigned id);
bool list_remove_node(struct t_list *list, struct t_list_node *node);
struct t_list_node *list_shift(struct t_list *list);

#endif

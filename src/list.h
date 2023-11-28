/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_LIST_H
#define MYGPIOD_LIST_H

#include <stdbool.h>

struct t_list_node {
    unsigned gpio;
    void *data;
    struct t_list_node *next;
};

struct t_list {
    struct t_list_node *head;
    struct t_list_node *tail;
    unsigned length;
};

typedef void (*list_data_clear)(void *node);

void list_init(struct t_list *list);
void list_clear(struct t_list *list, list_data_clear clear_data_callback);
bool list_push(struct t_list *list, unsigned gpio, void *data);
struct t_list_node *list_node_at(struct t_list *list, unsigned pos);
struct t_list_node *list_node_by_gpio(struct t_list *list, unsigned gpio);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 MYGPIOD (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mygpiod
*/

/*! \file
 * \brief Message queue implementation
 */

#ifndef MYGPIOD_QUEUE_H
#define MYGPIOD_QUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include <time.h>

extern struct t_mygpiod_queue *main_queue;
extern struct t_mygpiod_queue *script_queue;

/**
 * Typedef for free function for msg data
 */
typedef void (*t_data_free)(void *);

/**
 * A message in the queue
 */
struct t_mygpiod_msg {
    void *data;                  //!< Data
    t_data_free data_free;       //!< Function pointer to free the data pointer
    unsigned id;                 //!< id of the message
    time_t timestamp;            //!< messages added timestamp
    struct t_mygpiod_msg *next;  //!< pointer to next message
};

/**
 * Struct for the thread save message queue
 */
struct t_mygpiod_queue {
    unsigned length;                //!< length of the queue
    struct t_mygpiod_msg *head;     //!< pointer to first message
    struct t_mygpiod_msg *tail;     //!< pointer to last message
    pthread_mutex_t mutex;          //!< the mutex
    pthread_cond_t wakeup;          //!< condition variable for the mutex
    const char *name;               //!< descriptive name
    int event_fd;                   //!< event fd
};

struct t_mygpiod_queue *mygpiod_queue_create(const char *name, bool event);
void mygpiod_queue_free(struct t_mygpiod_queue *queue);
bool mygpiod_queue_push(struct t_mygpiod_queue *queue, void *data, t_data_free data_free, unsigned id);
struct t_mygpiod_msg *mygpiod_queue_shift(struct t_mygpiod_queue *queue, int timeout_ms, unsigned id);
int mygpiod_queue_expire_age(struct t_mygpiod_queue *queue, time_t max_age_s);
void free_queue_node(struct t_mygpiod_msg *node);

#endif

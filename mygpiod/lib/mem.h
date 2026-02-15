/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_MEM_H
#define MYGPIOD_MEM_H

#include "mygpiod/lib/log.h"

#include <assert.h>
#include <stdlib.h>

/**
 * Mallocs and asserts if it fails
 * @param size bytes to malloc
 * @return malloced pointer
 */
__attribute__((malloc))
static inline void *malloc_assert(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        MYGPIOD_LOG_EMERG(NULL, "Failure allocating %lu bytes of memory", (unsigned long) size);
        abort();
    }
    return p;
}

/**
 * Reallocs and asserts if it fails
 * @param ptr pointer to resize
 * @param size bytes to realloc
 * @return reallocated pointer
 */
__attribute__((malloc))
static inline void *realloc_assert(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (p == NULL) {
        MYGPIOD_LOG_EMERG(NULL, "Failure allocating %lu bytes of memory", (unsigned long) size);
        abort();
    }
    return p;
}

/**
 * Macro to free the pointer and set it to NULL
 * @param PTR pointer to free
 */
#define FREE_PTR(PTR) do { \
    if (PTR != NULL) \
        free(PTR); \
    PTR = NULL; \
} while (0)

#endif

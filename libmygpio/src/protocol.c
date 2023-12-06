/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/src/protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Checks for the OK response line
 * @param line line to parse
 * @return true on success, else false
 */
bool check_response_ok(const char *line) {
    if (strcmp(line, "OK") == 0) {
        return true;
    }
    return false;
}

/**
 * Checks for the END response line
 * @param line line to parse
 * @return true on success, else false
 */
bool check_response_end(const char *line) {
    if (strcmp(line, "END") == 0) {
        return true;
    }
    return false;
}

/**
 * Parses a line to a key/value pair.
 * Key and value are only pointers and are not copied.
 * @param line line to parse
 * @return allocated pair or NULL on error
 */
struct t_mygpio_pair *parse_pair(const char *line) {
    struct t_mygpio_pair *pair = malloc(sizeof(struct t_mygpio_pair));
    assert(pair);
    char *p = strchr(line, ':');
    if (p == NULL) {
        free_pair(pair);
        return NULL;
    }
    pair->name = line;
    *p = '\0';
    pair->value = p + 1;
    return pair;
}

/**
 * Frees the key/value pair
 * @param pair pair to free
 */
void free_pair(struct t_mygpio_pair *pair) {
    pair->name = NULL;
    pair->value = NULL;
    free(pair);
}

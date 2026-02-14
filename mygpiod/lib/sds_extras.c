/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/lib/sds_extras.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Getline function that trims whitespace characters.
 * @param s an already allocated sds string
 * @param fp a file descriptor to read from
 * @param max max line length to read
 * @param nread read bytes
 * @return Pointer to s
 */
sds sds_getline(sds s, FILE *fp, size_t max, int *nread) {
    sdsclear(s);
    s = sdsMakeRoomFor(s, max + 1);
    for (size_t i = 0; i < max; i++) {
        int c = fgetc(fp);
        if (c == EOF ||
            c == '\n')
        {
            s[i] = '\0';
            sdstrim(s, "\r \t");
            *nread = c == EOF
                ? -1
                : (int)sdslen(s);
            return s;
        }
        s[i] = (char)c;
        sdsinclen(s, 1);
    }
    MYGPIOD_LOG_ERROR("Line is too long, max length is %lu", (unsigned long)max);
    s[max] = '\0';
    sdstrim(s, "\r \t");

    *nread = (int)sdslen(s);
    return s;
}

/**
 * Reads a whole file in a sds string
 * @param s an already allocated sds string to append the file content
 * @param file_path filename to read
 * @param nread Number of bytes read,
 *              -1 error reading file
 * @return pointer to s
 */
sds sds_getfile(sds s, const char *file_path, int *nread) {
    errno = 0;
    FILE *fp = fopen(file_path, OPEN_FLAGS_READ);
    if (fp == NULL) {
        *nread = -1;
        MYGPIOD_LOG_ERROR("Unable to open file \"%s\": %s", file_path, strerror(errno));
        return s;
    }
    const size_t buffer_size = 10240;
    s = sdsMakeRoomFor(s, buffer_size);
    size_t read;
    while ((read = fread(s + sdslen(s), sizeof(char), buffer_size, fp)) > 0) {
        sdsIncrLen(s, (ssize_t)read);
        s = sdsMakeRoomFor(s, buffer_size);
    }
    if (ferror(fp)) {
        MYGPIOD_LOG_ERROR("Error reading file \"%s\"", file_path);
    }
    (void) fclose(fp);
    *nread = (int)sdslen(s);
    return s;
}

/**
 * Splits the string into two parts by first occurrence of sep.
 * Trims whitespaces from start and end of the tokens
 * @param s sds string to split
 * @param sep separator char
 * @param count pointer to populate the 
 * @return allocated array of tokens or NULL on error
 */
sds *sds_splitfirst(sds s, char sep, int *count) {
    size_t len = sdslen(s);
    sds *tokens = malloc_assert(sizeof(sds)*2);
    size_t start = 0;
    int elements = 0;
    for (size_t i = 0; i < len; i++) {
        if (*(s + i) == sep) {
            tokens[elements] = sdsnewlen(s, i);
            sdstrim(tokens[elements], " \t");
            elements++;
            start = i + 1;
            break;
        }
    }

    tokens[elements] = sdsnewlen(s + start, len - start);
    sdstrim(tokens[elements], " \t");
    elements++;
    *count = elements;
    return tokens;
}

/**
 * Appends a char to sds string s, this is faster than using sdscatfmt
 * @param s sds string
 * @param c char to append
 * @return modified sds string
 */
sds sds_catchar(sds s, const char c) {
    // Make sure there is always space for at least 1 char.
    s = sdsMakeRoomFor(s, 1);
    if (s == NULL) {
        return NULL;
    }
    size_t i = sdslen(s);
    s[i++] = c;
    sdsinclen(s, 1);
    // Add null-term
    s[i] = '\0';
    return s;
}

/**
 * Returns the string until separator is found and removes it from original string.
 * @param s String to modify
 * @param sep Seperatir
 * @return sds or NULL on error
 */
sds sds_getvalue(sds s, char sep) {
    if (sdslen(s) == 0) {
        return NULL;
    }
    size_t len = sdslen(s);
    size_t i = 0;
    for (; i < len; i++) {
        if (*(s + i) == sep) {
            break;
        }
    }
    sds value = i > 0
        ? sdsnewlen(s, i)
        : sdsempty();
    if (i > 0) {
        sdsrange(s, (ssize_t)i + 1, -1);
    }
    return value;
}

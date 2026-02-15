/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief sds related functions
 */

#ifndef MYGPIOD_SDS_EXTRAS_H
#define MYGPIOD_SDS_EXTRAS_H

#include "dist/sds/sds.h"

#include <gpiod.h>
#include <stddef.h>
#include <stdio.h>

/**
 * Free sds and set it to NULL
 */
#define FREE_SDS(SDS_PTR) do { \
    sdsfree(SDS_PTR); \
    SDS_PTR = NULL; \
} while (0)

sds sds_getline(sds s, FILE *fp, size_t max, int *nread);
sds sds_getfile(sds s, const char *file_path, int *nread);
sds *sds_splitfirst(sds s, char sep, int *count);
sds sds_catchar(sds s, const char c);
sds sds_getvalue(sds s, char sep);

#endif

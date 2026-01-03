/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SDS_EXTRAS_H
#define MYGPIOD_SDS_EXTRAS_H

#include "dist/sds/sds.h"

#include <gpiod.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define FREE_SDS(SDS_PTR) do { \
    sdsfree(SDS_PTR); \
    SDS_PTR = NULL; \
} while (0)

sds sds_getline(sds s, FILE *fp, size_t max, int *nread);
sds sds_getfile(sds s, const char *file_path, int *nread);
sds *sds_splitfirst(sds s, char sep, int *count);
sds sds_catchar(sds s, const char c);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2024-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOweb
*/

#ifndef JSON_H
#define JSON_H

#include "dist/sds/sds.h"

#include <stdbool.h>

sds sds_catjson_plain(sds s, const char *p);
sds sds_catjson(sds s, const char *p);
sds sds_catjson_plain_len(sds s, const char *p, size_t len);
sds sds_catjson_len(sds s, const char *p, size_t len);

const char *bool_to_str(bool v);

#endif

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOweb (c) 2024-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOweb
*/

#include "mygpiod/lib//json_print.h"

#include "dist/sds/sds.h"

#include <string.h>

/**
 * JSON escapes special chars
 * @param c char to escape
 * @return escaped char
 */
static const char *escape_char(char c) {
    switch(c) {
        case '\\': return "\\\\";
        case '"':  return "\\\"";
        case '\b': return "\\b";
        case '\f': return "\\f";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
    }
    return NULL;
}

/**
 * Append to the sds string "s" a json escaped string
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 * @param s sds string
 * @param p string to append json escaped
 * @return modified sds string
 */
sds sds_catjson_plain(sds s, const char *p) {
    return sds_catjson_plain_len(s, p, strlen(p));
}

/**
 * Append to the sds string "s" a json escaped string
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 * @param s sds string
 * @param p string to append json escaped
 * @param len length of the string to append
 * @return modified sds string
 */
sds sds_catjson_plain_len(sds s, const char *p, size_t len) {
    /* To avoid continuous reallocations, let's start with a buffer that
     * can hold at least stringlength + 10 chars. */
    s = sdsMakeRoomFor(s, len + 10);
    size_t i = sdslen(s);
    while (len--) {
        switch(*p) {
            case '\\':
            case '"':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t': {
                if (sdsavail(s) == 0) {
                    s = sdsMakeRoomFor(s, 2);
                }
                const char *escape = escape_char(*p);
                s[i++] = escape[0];
                s[i++] = escape[1];
                sdsinclen(s, 2);
                break;
            }
            //ignore vertical tabulator and alert
            case '\v':
            case '\a':
                //this escapes are not accepted in the unescape function
                break;
            default:
                if (sdsavail(s) == 0) {
                    s = sdsMakeRoomFor(s, 1);
                }
                s[i++] = *p;
                sdsinclen(s, 1);
                break;
        }
        p++;
    }
    // Add null-term
    s[i] = '\0';
    return s;
}

/**
 * Append to the sds string "s" a quoted json escaped string
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 * @param s sds string
 * @param p string to append json escaped
 * @return modified sds string
 */
sds sds_catjson(sds s, const char *p) {
    return sds_catjson_len(s, p, strlen(p));
}

/**
 * Append to the sds string "s" a quoted json escaped string
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 * @param s sds string
 * @param p string to append json escaped
 * @param len length of the string to append
 * @return modified sds string
 */
sds sds_catjson_len(sds s, const char *p, size_t len) {
    /* To avoid continuous reallocations, let's start with a buffer that
     * can hold at least stringlength + 10 chars. */
    s = sdsMakeRoomFor(s, len + 10);
    s = sdscatlen(s, "\"", 1);
    s = sds_catjson_plain_len(s, p, len);
    return sdscatlen(s, "\"", 1);
}

/**
 * Returns the string representation for a bool value.
 * @param v bool value
 * @return bool value as string literal
 */
const char *bool_to_str(bool v) {
    return v == true
        ? "true"
        : "false";
}

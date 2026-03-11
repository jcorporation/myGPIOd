/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief HTTP actions
 */

#include "compile_time.h"
#include "mygpiod/actions/http.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/http_client.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// private definitions

/**
 * List of valid HTTP methods
 */
const char *http_methods[] = {
    "DELETE",
    "GET",
    "HEAD",
    "OPTIONS",
    "PATCH",
    "POST",
    "PUT",
    NULL
};

/**
 * Arguments for the curl thread
 */
struct t_curl_arguments {
    sds method;          //!< HTTP method
    sds uri;             //!< URI
    sds content_type;    //!< Content type of post data
    sds postdata;        //!< Post data
};

static void free_curl_arguments(struct t_curl_arguments *arg);
static bool validate_http_method(const char *method);
static void *curl_thread(void *thread_arg);

// public functions

/**
 * Makes a http call in a new thread.
 * @param action Action struct, options must be:
 *               {method} {uri} [{content-type} {postdata}]
 * @returns true on success, else false
 */
bool action_http_async(struct t_action *action) {
    bool rc = false;
    if (action->options_count == 4) {
        // Request with body
        rc = action_http2_async(action->options[0], action->options[1], action->options[2], action->options[3]);
    }
    else if (action->options_count == 2) {
        // Request without body
        rc = action_http2_async(action->options[0], action->options[1], NULL, NULL);
    }
    else {
        MYGPIOD_LOG_ERROR("Invalid number of arguments: %d", action->options_count);
    }
    return rc;
}

/**
 * Makes a http call in a new thread
 * @param method HTTP method
 * @param uri HTTP Uri
 * @param content_type Content-Type or NULL
 * @param postdata data to post or NULL
 * @returns true on success, else false
 */
bool action_http2_async(const char *method, const char *uri, const char *content_type, const char *postdata) {
    if (validate_http_method(method) == false) {
        MYGPIOD_LOG_ERROR("Invalid HTTP method: \"%s\"", method);
        return false;
    }
    struct t_curl_arguments *arg = malloc_assert(sizeof(struct t_curl_arguments));
    arg->method = sdsnew(method);
    arg->uri = sdsnew(uri);
    if (content_type != NULL &&
        postdata != NULL)
    {
        arg->content_type = sdsnew(content_type);
        arg->postdata = sdsnew(postdata);
    }
    else {
        arg->content_type = NULL;
        arg->postdata = NULL;
    }

    pthread_t scripts_worker_thread;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0 ||
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 ||
        pthread_create(&scripts_worker_thread, &attr, curl_thread, arg) != 0)
    {
        MYGPIOD_LOG_ERROR("Can not create http client thread");
        free_curl_arguments(arg);
        return false;
    }
    return true;
}

// private functions

/**
 * Frees a t_curl_arguments struct
 * @param arg t_curl_arguments struct to free
 */
static void free_curl_arguments(struct t_curl_arguments *arg) {
    sdsfree(arg->content_type);
    sdsfree(arg->method);
    sdsfree(arg->postdata);
    sdsfree(arg->uri);
    free(arg);
}

/**
 * Checks if string is a valid HTTP method
 * @param method Method string to parse
 * @return true if it is valid HTTP method, else false
 */
static bool validate_http_method(const char *method) {
    const char **p = http_methods;
    while(*p != NULL) {
        if (strcmp(method, *p) == 0) {
            return true;
        }
        p++;
    }
    return false;
}

/**
 * Main function for action_http2_async
 * @param thread_arg Void pointer to struct t_curl_arguments
 * @returns NULL
 */
static void *curl_thread(void *thread_arg) {
    logline = sdsempty();
    struct t_curl_arguments *arg = (struct t_curl_arguments *)thread_arg;

    sds resp_header = sdsempty();
    sds resp_body = sdsempty();
    http_client(arg->method, arg->uri, arg->content_type, arg->postdata, &resp_header, &resp_body);

    sdstrim(resp_header, "\r\n");
    sdstrim(resp_body, "\r\n");
    resp_header = sdsmapchars(resp_header, "\r\n", "  ", 2);
    resp_body = sdsmapchars(resp_body, "\r\n", "  ", 2);
    if (sdslen(resp_header) > 1023) {
        sdsrange(resp_header, 0, 1020);
        resp_header = sdscatlen(resp_header, "...", 3);
    }
    if (sdslen(resp_body) > 1023) {
        sdsrange(resp_body, 0, 1020);
        resp_body = sdscatlen(resp_body, "...", 3);
    }
    MYGPIOD_LOG_DEBUG("Header: %s\nBody: %s", resp_header, resp_body);

    sdsfree(resp_header);
    sdsfree(resp_body);
    free_curl_arguments(arg);
    sdsfree(logline);
    return NULL;
}

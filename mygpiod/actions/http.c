/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/http.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/sds_extras.h"

#include <curl/curl.h>
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
    sds method;
    sds uri;
    sds content_type;
    sds postdata;
};

static void free_curl_arguments(struct t_curl_arguments *arg);
static bool validate_http_method(const char *method);
static void *curl_thread(void *thread_arg);
static size_t catch_output(void *ptr, size_t size, size_t nmemb, sds *output);

// public functions

/**
 * Makes a http call in a new process. Parses the cmd.
 * @param cmd command and it's options, format:
 *            {method} {uri} [{content-type} {postdata}]
 * @returns true on success, else false
 */
bool action_http(struct t_action *action) {
    bool rc = false;
    if (action->options_count == 4) {
        // Request with body
        rc = action_http2(action->options[0], action->options[1], action->options[2], action->options[3]);
    }
    else if (action->options_count == 2) {
        // Request without body
        rc = action_http2(action->options[0], action->options[1], NULL, NULL);
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
bool action_http2(const char *method, const char *uri, const char *content_type, const char *postdata) {
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
 * Makes an HTTP call
 * @param method HTTP method
 * @param uri HTTP Uri
 * @param content_type Content-Type or NULL
 * @param postdata data to post or NULL
 * @return true on success, else false
 */
static void *curl_thread(void *thread_arg) {
    // Initialize
    logline = sdsempty();
    struct t_curl_arguments *arg = (struct t_curl_arguments *)thread_arg;
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT;
    sds resp_header = sdsempty();
    sds resp_body = sdsempty();

    // Run curl
    curl = curl_easy_init();
    if (curl == NULL) {
        goto out;
    }
    curl_easy_setopt(curl, CURLOPT_URL, arg->uri);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (curl_write_callback)catch_output);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_body);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp_header);
    struct curl_slist *slist = NULL;
    if (arg->postdata != NULL) {
        sds header = sdscatfmt(sdsempty(), "Content-type: %s", arg->content_type);
        slist = curl_slist_append(slist, header);
        if (slist == NULL) {
            goto out;
        }
        sdsfree(header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
        if (strncmp(arg->postdata, "<</", 3) == 0) {
            // read file
            sds file_path = sdsdup(arg->postdata);
            sdsrange(file_path, 2, -1);
            sdsclear(arg->postdata);
            int nread;
            arg->postdata = sds_getfile(arg->postdata, file_path, &nread);
            sdsfree(file_path);
            if (nread == -1) {
                goto out;
            }
        }
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, arg->postdata);
    }
    char err_buf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_buf);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, arg->method);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "myGPIOd");
    res = curl_easy_perform(curl);
    curl_slist_free_all(slist);
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
    if (res != CURLE_OK) {
        MYGPIOD_LOG_ERROR("HTTP call failed: %s", curl_easy_strerror(res));
        MYGPIOD_LOG_ERROR("Error: %s", err_buf);
        MYGPIOD_LOG_ERROR("Header: %s\nBody: %s", resp_header, resp_body);
    }
    else {
        MYGPIOD_LOG_DEBUG("Header: %s\nBody: %s", resp_header, resp_body);
    }

    // Cleanup
    out:
    if (curl != NULL) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    sdsfree(resp_header);
    sdsfree(resp_body);
    free_curl_arguments(arg);
    sdsfree(logline);
    return NULL;
}

/**
 * Callback function for curl
 * @param ptr char pointer with data
 * @param size always 1
 * @param nmemb size of the data
 * @param output already allocated sds string to append the data
 * @return bytes appended to output
 */
static size_t catch_output(void *ptr, size_t size, size_t nmemb, sds *output) {
    (void)size;
    *output = sdscatlen(*output, ptr, nmemb);
    return nmemb;
}

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/http.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/util.h"

#include <curl/curl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// private definitions
static bool fork_call_curl(const char *method, const char *uri, const char *content_type, const char *postdata);
static bool call_curl(const char *method, const char *uri, const char *content_type, const char *postdata);
static size_t catch_output(void *ptr, size_t size, size_t nmemb, sds *output);

// public functions

/**
 * Makes a http call in a new process. Parses the cmd.
 * @param cmd command and it's options, format:
 *            {GET|POST} {uri} [{content-type} {postdata}]
 * @returns true on success, else false
 */
bool action_http(const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);
    if (count < 2) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        sdsfreesplitres(args, count);
        return false;
    }
    bool rc = false;
    if (strcasecmp(args[0], "post") == 0) {
        if (count != 4) {
            MYGPIOD_LOG_ERROR("Invalid number of arguments");
            sdsfreesplitres(args, count);
            return false;
        }
        rc = fork_call_curl(args[0], args[1], args[2], args[3]);
    }
    else {
        rc = fork_call_curl(args[0], args[1], NULL, NULL);
    }
    sdsfreesplitres(args, count);
    return rc;
}

/**
 * Makes a http call in a new process
 * @param method HTTP method: POST or GET
 * @param uri HTTP Uri
 * @param content_type Content-Type or NULL
 * @param postdata data to post or NULL
 * @returns true on success, else false
 */
bool action_http2(const char *method, const char *uri, const char *content_type, const char *postdata) {
    bool rc = false;
    if (strcasecmp(method, "post") == 0) {
        if (content_type == NULL ||
            postdata == NULL)
        {
            MYGPIOD_LOG_ERROR("Invalid number of arguments");
            return false;
        }
        rc = fork_call_curl(method, uri, content_type, postdata);
    }
    else {
        rc = fork_call_curl(method, uri, NULL, NULL);
    }
    return rc;
}

// private functions

/**
 * Forks and makes the http call
 * @param method HTTP method: POST or GET
 * @param uri HTTP Uri
 * @param content_type Content-Type or NULL
 * @param postdata data to post or NULL
 * @return true on success, else false
 */
static bool fork_call_curl(const char *method, const char *uri, const char *content_type, const char *postdata) {
    errno = 0;
    int pid = fork();
    if (pid == 0) {
        // this is the child process
        bool rc = call_curl(method, uri, content_type, postdata);
        if (rc == true) {
            exit(EXIT_SUCCESS);
        }
        exit(EXIT_FAILURE);
    }
    else {
        if (pid == -1) {
            MYGPIOD_LOG_ERROR("Could not fork: %s", strerror(errno));
            return false;
        }
        MYGPIOD_LOG_DEBUG("Forked process with pid %d", pid);
        return true;
    }
}

/**
 * Makes an HTTP call
 * @param method HTTP method: POST or GET
 * @param uri HTTP Uri
 * @param content_type Content-Type or NULL
 * @param postdata data to post or NULL
 * @return true on success, else false
 */
static bool call_curl(const char *method, const char *uri, const char *content_type, const char *postdata) {
    (void) method;
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        curl_global_cleanup();
        return false;
    }
    sds resp_header = sdsempty();
    sds resp_body = sdsempty();
    sds data = sdsnew(postdata);
    curl_easy_setopt(curl, CURLOPT_URL, uri);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (curl_write_callback)catch_output);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_body);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp_header);
    struct curl_slist *slist = NULL;
    if (postdata != NULL) {
        sds header = sdscatfmt(sdsempty(), "Content-type: %s", content_type);
        slist = curl_slist_append(slist, header);
        if (slist == NULL) {
            return false;
        }
        sdsfree(header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
        if (strncmp(postdata, "<</", 3) == 0) {
            // read file
            sds file_path = sdsnew(postdata);
            sdsrange(file_path, 2, -1);
            sdsclear(data);
            int nread;
            data = sds_getfile(data, file_path, &nread);
            sdsfree(file_path);
            if (nread == -1) {
                return false;
            }
        }
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    }
    char err_buf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_buf);
    CURLcode res = curl_easy_perform(curl);
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
        if (sdslen(resp_header) > 0) {
            MYGPIOD_LOG_ERROR("%s", resp_header);
        }
        if (sdslen(resp_body) > 0) {
            MYGPIOD_LOG_ERROR("%s", resp_body);
        }
    }
    else {
        if (sdslen(resp_header) > 0) {
            MYGPIOD_LOG_DEBUG("%s", resp_header);
        }
        if (sdslen(resp_body) > 0) {
            MYGPIOD_LOG_DEBUG("%s", resp_body);
        }
    }
    curl_global_cleanup();
    sdsfree(resp_header);
    sdsfree(resp_body);
    sdsfree(data);
    if (res != CURLE_OK) {
        return false;
    }
    return true;
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

/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
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
static size_t catch_output(void *ptr, size_t size, size_t nmemb, sds *output);
static void run_http(const char *cmd);

// public functions

/**
 * Makes a http call in a new process
 * @param cmd command to parse
 * @returns true on success, else false
 */
bool action_http(const char *cmd) {
    errno = 0;
    int pid = fork();
    if (pid == 0) {
        // this is the child process
        run_http(cmd);
        exit(EXIT_SUCCESS);
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

// private functions

/**
 * Connects to MPD and runs the command
 * @param cmd command and it's options, format:
 *            {GET|POST} {uri} [{content-type} {postdata}]
 * @return true on success, else false
 */
static void run_http(const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);
    if (count < 2) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        sdsfreesplitres(args, count);
        exit(EXIT_FAILURE);
    }
    bool has_body = false;
    if (strcasecmp(args[0], "post") == 0) {
        has_body = true;
        if (count != 4) {
            MYGPIOD_LOG_ERROR("Invalid number of arguments");
            sdsfreesplitres(args, count);
            exit(EXIT_FAILURE);
        }
    }

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        curl_global_cleanup();
        sdsfreesplitres(args, count);
        exit(EXIT_FAILURE);
    }
    sds resp_header = sdsempty();
    sds resp_body = sdsempty();
    curl_easy_setopt(curl, CURLOPT_URL, args[1]);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (curl_write_callback)catch_output);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_body);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp_header);
    struct curl_slist *slist = NULL;
    if (has_body == true) {
        sds header = sdscatfmt(sdsempty(), "Content-type: %s", args[2]);
        slist = curl_slist_append(slist, header);
        if (slist == NULL) {
            exit(EXIT_FAILURE);
        }
        sdsfree(header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
        if (strncmp(args[3], "<</", 3) == 0) {
            // read file
            sds file_path = sdsdup(args[3]);
            sdsrange(file_path, 2, -1);
            sdsclear(args[3]);
            int nread;
            args[3] = sds_getfile(args[3], file_path, &nread);
            sdsfree(file_path);
            if (nread == -1) {
                exit(EXIT_FAILURE);
            }
        }
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args[3]);
    }
    char err_buf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_buf);
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(slist);
    sdstrim(resp_header, "\r\n");
    sdstrim(resp_body, "\r\n");
    resp_header = sdsmapchars(resp_header, "\r\n", "  ", 2);
    resp_body = sdsmapchars(resp_body, "\r\n", "  ", 2);
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
    sdsfree(resp_header);
    sdsfree(resp_body);
    curl_global_cleanup();
    sdsfreesplitres(args, count);
    if (res != CURLE_OK) {
        exit(EXIT_FAILURE);
    }
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

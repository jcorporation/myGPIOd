/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief HTTP client implementation
 */

#include "compile_time.h"
#include "mygpiod/lib/http_client.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/sds_extras.h"

#include <curl/curl.h>
#include <stddef.h>
#include <string.h>

// Private definitions

static size_t catch_output(void *ptr, size_t size, size_t nmemb, sds *output);

// Public functions

/**
 * Makes a http call in a new thread
 * @param method HTTP method
 * @param uri HTTP Uri
 * @param content_type Content-Type or NULL
 * @param postdata data to post or NULL
 * @returns true on success, else false
 */
bool http_client(const char *method, const char *uri, const char *content_type, const char *postdata,
        sds *response_header, sds *response_body)
{
    // Initialize
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT;
    bool rc = false;
    // Run curl
    curl = curl_easy_init();
    if (curl == NULL) {
        goto out;
    }
    curl_easy_setopt(curl, CURLOPT_URL, uri);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (curl_write_callback)catch_output);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
    struct curl_slist *slist = NULL;
    if (postdata != NULL) {
        sds header = sdscatfmt(sdsempty(), "Content-type: %s", content_type);
        slist = curl_slist_append(slist, header);
        if (slist == NULL) {
            goto out;
        }
        sdsfree(header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
        if (strncmp(postdata, "<</", 3) == 0) {
            // read file
            sds file_path = sdsnew(postdata);
            sdsrange(file_path, 2, -1);
            int nread;
            sds file_content = sds_getfile(sdsempty(), file_path, &nread);
            sdsfree(file_path);
            if (nread == -1) {
                goto out;
            }
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, file_content);
        }
        else {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
        }
    }
    char err_buf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_buf);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "myGPIOd");
    res = curl_easy_perform(curl);
    curl_slist_free_all(slist);
    if (res != CURLE_OK) {
        MYGPIOD_LOG_ERROR("HTTP call failed: %s", curl_easy_strerror(res));
        MYGPIOD_LOG_ERROR("Error: %s", err_buf);
    }
    else {
        rc = true;
    }

    // Cleanup
    out:
    if (curl != NULL) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return rc;
}

// Private functions

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

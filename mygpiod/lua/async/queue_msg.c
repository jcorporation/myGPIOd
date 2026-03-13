/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Message handling from async lua thread
 */

#include "compile_time.h"
#include "mygpiod/lua/async/queue_msg.h"

#include "mygpiod/event_loop/eventfd_wrap.h"
#include "mygpiod/event_loop/msg_queue.h"
#include "mygpiod/lib/mem.h"

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Reads the message from the main_queue and executes the requested Lua function.
 * Pushes the number of values on the Lua stack back to the script_queue.
 * @param fd Pointer to evenfd to read from
 * @return true on success, else false
 */
bool lua_async_handle_msg(int *fd) {
    if (event_eventfd_read(*fd) == false) {
        return false;
    }
    struct t_mygpiod_msg *msg = mygpiod_queue_shift(main_queue, -1, 0);
    struct t_lua_async_request *req = (struct t_lua_async_request *)msg->data;
    // Execute Lua function in main thread
    int rc = req->lua_func(req->lua_vm);
    // Respond
    struct t_lua_async_response *resp = lua_async_response_new(rc);
    mygpiod_queue_push(script_queue, resp, lua_async_response_free, msg->id);
    free_queue_node(msg);
    return true;
}

/**
 * Sends the message from the Lua thread to the main_queue and waits for the
 * response.
 * @param lua_vm Pointer to Lua VM.
 * @param lua_func Pointer to function that should be executed in the main thread.
 * @return Number of values on the Lua stack, or 0 on error.
 */
int lua_async_send_msg(lua_State *lua_vm, t_lua_func lua_func) {
    struct t_lua_async_request *req = lua_async_request_new(lua_vm, lua_func);
    unsigned request_id = (unsigned)syscall(SYS_gettid);
    mygpiod_queue_push(main_queue, req, lua_async_request_free, request_id);
    struct t_mygpiod_msg *msg = mygpiod_queue_shift(script_queue, 0, request_id);
    if (msg == NULL) {
        MYGPIOD_LOG_ERROR("Invalid response for lua_async_send_msg");
        return 0;
    }
    struct t_lua_async_response *resp = (struct t_lua_async_response *)msg->data;
    int rc = resp->rc;
    msg->data_free(msg->data);
    MYGPIOD_LOG_DEBUG("Lua function finished, pushed %d values on stack", rc);
    return rc;
}

/**
 * Creates a new request struct.
 * @param lua_vm Pointer to Lua VM
 * @param lua_func Pointer to function to execute
 * @return struct t_lua_async_request* 
 */
struct t_lua_async_request *lua_async_request_new(lua_State *lua_vm, t_lua_func lua_func) {
    struct t_lua_async_request *req = malloc_assert(sizeof(struct t_lua_async_request));
    req->lua_vm = lua_vm;
    req->lua_func = lua_func;
    return req;
}

/**
 * Frees the request struct.
 * @param data Pointer to data to free.
 */
void lua_async_request_free(void *data) {
    struct t_lua_async_request *req = (struct t_lua_async_request *)data;
    free(req);
}

/**
 * Creates a new response struct.
 * @param rc Number of values pushed to Lua stack.
 * @return struct t_lua_async_response* 
 */
struct t_lua_async_response *lua_async_response_new(int rc) {
    struct t_lua_async_response *resp = malloc_assert(sizeof(struct t_lua_async_response));
    resp->rc = rc;
    return resp;
}

/**
 * Frees the response struct.
 * @param data Pointer to data to free.
 */
void lua_async_response_free(void *data) {
    struct t_lua_async_response *resp = (struct t_lua_async_response *)data;
    free(resp);
}

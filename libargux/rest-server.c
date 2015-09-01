/*-
 * Copyright (c) 2015 Stephan Arts. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY [LICENSOR] "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE,DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#include <unistd.h>

#include <string.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>


#include <microhttpd.h>

#include "memory.h"
#include "log.h"
#include "rest-server.h"

#define API_VERSION 1

static void
_http_request_completed (
        void *cls,
        struct MHD_Connection *connection,
        void **con_cls,
        enum MHD_RequestTerminationCode toe);

static int
_http_handle_request (
        void *cls,
        struct MHD_Connection *connection, 
        const char *url, 
        const char *method,
        const char *version, 
        const char *upload_data, 
        size_t *upload_data_size,
        void **con_cls);

struct _ArguxRestServer
{
    struct MHD_Daemon *daemon;

    void (*cb_get)(const char *url);
};

ArguxRestServer *
argux_rest_server_start(int port, int api_version)
{

    ArguxRestServer *server = argux_new (
            sizeof (ArguxRestServer),
            1);

    server->daemon = NULL;

    struct MHD_Daemon *daemon = MHD_start_daemon (
            MHD_USE_DEBUG | MHD_USE_SELECT_INTERNALLY,
            port,
            NULL, NULL, 
            &_http_handle_request, server,
            MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
            MHD_OPTION_NOTIFY_COMPLETED, _http_request_completed, NULL,
            MHD_OPTION_END);

    if (NULL == daemon) {
        free (server);
        return NULL;
    }

    server->daemon = daemon;

    server->cb_get = NULL;

    return server;
}

void
argux_rest_server_stop(ArguxRestServer *server)
{
    MHD_stop_daemon (server->daemon);

    free(server);
}

static void
_http_request_completed (
        void *cls,
        struct MHD_Connection *connection,
        void **con_cls,
        enum MHD_RequestTerminationCode toe)
{
    if (*con_cls!= NULL) {
        argux_log_debug("COMPLETED (and cleaned up)");
        free (*con_cls);
    }
}



static int
_http_handle_request (
        void *cls,
        struct MHD_Connection *connection, 
        const char *url, 
        const char *method,
        const char *version, 
        const char *upload_data, 
        size_t *upload_data_size,
        void **con_cls)
{
    struct MHD_Response *response;
    int ret;

    /* Check if daemon is initialised */
    ArguxRestServer *server = cls;
    if (server->daemon == NULL) {
        argux_log_debug ("Received request before daemon was properly initialised");
        return MHD_NO;
    }

  
    /* Never respond to the first call */
    if (*con_cls == NULL) {
        *con_cls = (void *)malloc(1);
        return MHD_YES;
    }

    if (0 != strcmp(method, MHD_HTTP_METHOD_PUT)) {
        //return MHD_queue_response (connection,
        //        400,
        //        bad_request_response);
        return MHD_NO;
    }

    if (0 != strcmp(method, MHD_HTTP_METHOD_GET)) {
        return MHD_NO;
    }


    if (0 != strcmp(method, MHD_HTTP_METHOD_POST)) {
        //return MHD_queue_response (connection,
        //        400,
        //        bad_request_response);
        return MHD_NO;
    }

    if (0 != strcmp(method, MHD_HTTP_METHOD_DELETE)) {
        //return MHD_queue_response (connection,
        //        400,
        //        bad_request_response);
        return MHD_NO;
    }


  
    /*
      response = MHD_create_response_from_buffer (strlen (page),
                                                (void*) page, MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
    */

  return ret;
}

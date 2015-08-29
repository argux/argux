/*-
 * Copyright (c) 2012-2014 Stephan Arts. All Rights Reserved.
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

#include <pthread.h>
#include <errno.h>

#include <time.h>

#include <semaphore.h>

#include <microhttpd.h>

#include <zmq.h>

#include <libargux/libargux.h>

#include "worker.h"
#include "settings.h"
#include "db.h"
#include "command.h"

/** Define 10 Second interval */
#define INTERVAL 10

#define INVALID_REQUEST_PAGE "<html><head><title>shit</title></head></html>"

void   *ctx = NULL;

#define         BUFFER_LEN      1024
char    buffer[BUFFER_LEN];

#define PORT 8888

void request_completed (void *cls,
                        struct MHD_Connection *connection,
                        void **con_cls,
                        enum MHD_RequestTerminationCode toe);
int handle_request (void *cls, struct MHD_Connection *connection, 
                    const char *url, 
                    const char *method, const char *version, 
                    const char *upload_data, 
                    size_t *upload_data_size, void **con_cls);

static struct MHD_Response *invalid_request_response;


void
argux_scheduler_main (int port, int n_workers)
{
    int     i = 0;
    int     no_linger = 0;
    int     ret = 0;
    void   *plugins;
    void   *controller;

    invalid_request_response = MHD_create_response_from_buffer (
            strlen (INVALID_REQUEST_PAGE),
            (void *) INVALID_REQUEST_PAGE,
            MHD_RESPMEM_PERSISTENT);


    pthread_t *workers = NULL;

    struct MHD_Daemon *daemon;

    if (ctx != NULL)
    {
        return;
    }

    daemon = MHD_start_daemon (
            MHD_USE_DEBUG | MHD_USE_SELECT_INTERNALLY,
            PORT,
            NULL, NULL, 
            &handle_request, NULL,
            MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
            MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL,
            MHD_OPTION_END);

    if (NULL == daemon) return;

    ctx = zmq_ctx_new();

    plugins = zmq_socket (ctx, ZMQ_REP);
    controller = zmq_socket (ctx, ZMQ_ROUTER);

    zmq_bind (plugins, "inproc://workers");
    zmq_bind (controller, "inproc://controller");

    workers = argux_new (sizeof (pthread_t), n_workers);

    for (i = 0; i < n_workers; ++i)
    {
        argux_worker_thread_new (
                &workers[i],
                ctx);
    }

    /**
     * Enter the main loop, schedules jobs in the queue
     *
     * NOTE:
     * Hmm, the queue is still missing
     */
    while (1)
    {
        char    msg[256];
        zmq_pollitem_t items[] = {
            {plugins, 0, ZMQ_POLLIN, 0},
            {controller, 0, ZMQ_POLLIN, 0},
        };
        zmq_poll (items, 2, -1);

        /* Scheduling of worker threads */
        if (items[0].revents & ZMQ_POLLIN)
        {
            int size = zmq_recv (plugins, msg, 255, 0);
            if (size != -1)
            {
                /* Process task */
                if (!strncmp (msg, "GET-COMMAND", 11))
                {
                    snprintf (msg, 255, "WAIT[%d]%n", INTERVAL, &size);
                    zmq_send (plugins, msg, size, 0);

                    if (0)
                    {
                        snprintf (msg, 255, "PROCESS%n", &size);
                        zmq_send (plugins, msg, size, 0);
                    }
                    continue;
                }

                /*
                 * Check if it is a message indicating a worker thread is
                 * finished processing.
                 */
                ret = sscanf (msg, "FINISH");
                if (ret == 1)
                {
                    argux_log_debug ("Worker Finished");
                    zmq_send (plugins, "-", 1, 0);
                    continue;
                }
                argux_log_debug ("Send 'a': '%s'", msg);
                zmq_send (plugins, "a", 1, 0);
            }
        }
        /* Listen to Controller socket for STOP message */
        if (items[1].revents & ZMQ_POLLIN)
        {
            int     size = zmq_recv (controller, msg, 255, 0);
            if (size != -1)
            {
                if (strncmp (msg, "STOP", 4))
                {
                    argux_log_debug ("Terminating main loop");

                    /** TODO:
                     * Use another mechanism to properly end threads.
                     */

                    for (i = 0; i < n_workers; ++i)
                    {
                        pthread_cancel (workers[i]);
                        pthread_join (workers[i], NULL);
                    }
                    break;
                }
            }
        }
    }

    zmq_setsockopt (
            controller,
            ZMQ_LINGER,
            &no_linger,
            sizeof (no_linger));
    zmq_close (controller);

    zmq_setsockopt (
            plugins,
            ZMQ_LINGER,
            &no_linger,
            sizeof (no_linger));
    zmq_close (plugins);

    ctx = NULL;

    MHD_stop_daemon (daemon);

}

void
argux_scheduler_main_quit ()
{
    void   *socket = zmq_socket (ctx, ZMQ_REQ);
    int     ret = 0;

    argux_log_debug ("Terminating %s", PACKAGE_NAME);

    zmq_connect (socket, "inproc://controller");

    ret = zmq_send (socket, "STOP", 4, 0);
    if (ret == -1)
    {
        argux_log_error ("Failed to send termination message");
    }
    zmq_close (socket);
} 
 
void request_completed (void *cls,
                        struct MHD_Connection *connection,
                        void **con_cls,
                        enum MHD_RequestTerminationCode toe)
{
    if (*con_cls!= NULL) {
        argux_log_debug("COMPLETED (and cleaned up)");
        free (*con_cls);
    }
}



int handle_request (void *cls, struct MHD_Connection *connection, 
                    const char *url, 
                    const char *method, const char *version, 
                    const char *upload_data, 
                    size_t *upload_data_size, void **con_cls)
{
  const char *page  = "<html><body>Hello, browser!</body></html>";
  struct MHD_Response *response;
  int ret;

  if (*con_cls == NULL) {
    *con_cls = (void *)malloc(1);
    return MHD_YES;
  }

  if (0 != strcmp(method, MHD_HTTP_METHOD_GET)) {
    return MHD_queue_response (connection,
            405,
            invalid_request_response);

    return MHD_NO;
  }

  
  response = MHD_create_response_from_buffer (strlen (page),
                                            (void*) page, MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

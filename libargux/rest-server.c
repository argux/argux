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

#include <openssl/sha.h>

#include <microhttpd.h>

#include "memory.h"
#include "log.h"
#include "principal.h"
#include "rest-server.h"
#include "session.h"

#define API_VERSION 1

#define COOKIE_NAME "Argux"

#define DENIED "<html><head><title>libmicrohttpd demo</title></head><body>Access denied</body></html>"

#define MY_OPAQUE_STR "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"

typedef struct _Connection Connection;

struct _Connection
{
    char cookie[256];
};

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

    ArguxCreateCallback cb_create;
    ArguxReadCallback   cb_read;

    ArguxLookupPrincipalCallback cb_lookup_principal;
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

    server->cb_create = NULL;
    server->cb_read   = NULL;
    server->cb_lookup_principal = NULL;

    return server;
}

void
argux_rest_server_stop(ArguxRestServer *server)
{
    MHD_stop_daemon (server->daemon);

    free(server);
}

int
argux_rest_server_set_create_cb (
        ArguxRestServer *server,
        ArguxCreateCallback cb_create)
{
    server->cb_create = cb_create;

    return 0;
}

int
argux_rest_server_set_read_cb (
        ArguxRestServer *server,
        ArguxReadCallback cb_read)
{
    server->cb_read = cb_read;

    return 0;
}

int
argux_rest_server_set_lookup_principal_cb (
        ArguxRestServer *server,
        ArguxLookupPrincipalCallback cb_lookup)
{
    server->cb_lookup_principal = cb_lookup;

    return 0;
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

int print_out_key (void *cls, enum MHD_ValueKind kind, 
                   const char *key, const char *value)
{
    Connection *con = cls;

    if (0 == strcmp(COOKIE_NAME, key)) {
        printf("%s:%s\n", key, value);    
        strncpy(con->cookie, value, 256);
    }
    return MHD_YES;
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
    char cookie[512];
    char cookie_val[SHA256_DIGEST_LENGTH*2+1];
    char *username;

    const char *realm = "test@argux.github.io";

    ArguxPrincipal *principal = NULL;

    Connection *priv_con = *con_cls;

    /* Check if daemon is initialised */
    ArguxRestServer *server = cls;
    if (server->daemon == NULL) {
        argux_log_debug ("Received request before daemon was properly initialised");
        return MHD_NO;
    }

  
    /* Never respond to the first call */
    if (*con_cls == NULL) {
        *con_cls = (void *)malloc(sizeof (Connection));
        ((Connection *)*con_cls)->cookie[0] = '\0';
        return MHD_YES;
    }

    /* Lookup Principal */
    if (server->cb_lookup_principal != NULL) {

        /* Retrieve digest authentication username */
        username = MHD_digest_auth_get_username(connection);
        if (username == NULL)
        {
            response = MHD_create_response_from_buffer(strlen (DENIED),
                             DENIED,
                             MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_auth_fail_response(connection, realm,
                             MY_OPAQUE_STR,
                             response,
                             MHD_NO);
            MHD_destroy_response(response);
            return ret;
        }

        server->cb_lookup_principal(username, &principal);

        /** If the user does not exist, respond */
        if (principal == NULL) {

            response = MHD_create_response_from_buffer(strlen (DENIED),
                             DENIED,
                             MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_auth_fail_response(connection, realm,
                             MY_OPAQUE_STR,
                             response,
                             MHD_NO);
            MHD_destroy_response(response);
            return ret;
        }


        ret = MHD_digest_auth_check (connection, realm,
                username,
                "1234",
                300);
        if ( (ret == MHD_INVALID_NONCE) ||
             (ret == MHD_NO) )
        {
            response = MHD_create_response_from_buffer(strlen (DENIED),
                             DENIED,
                             MHD_RESPMEM_PERSISTENT);
            if (NULL == response)
                return MHD_NO;
            ret = MHD_queue_auth_fail_response(connection, realm,
                         MY_OPAQUE_STR,
                         response,
                         (ret == MHD_INVALID_NONCE) ? MHD_YES : MHD_NO);
            MHD_destroy_response(response);
            return ret;
        }
    }


    MHD_get_connection_values (connection, MHD_COOKIE_KIND, &print_out_key, *con_cls);
    if (strcmp(priv_con->cookie, "") == 0)
    {
        response = MHD_create_response_from_buffer(23, "<h1>Created-Cookie</h1>", MHD_RESPMEM_MUST_COPY);
        argux_sessionid_generate(cookie_val);
        ret = snprintf(cookie, 512, "%s=%s;HttpOnly", COOKIE_NAME, cookie_val);
        MHD_add_response_header (response, "SET-COOKIE", cookie);
    } else {
        response = MHD_create_response_from_buffer(19, "<h1>HAZ-Cookie</h1>", MHD_RESPMEM_MUST_COPY);
    }

    return MHD_queue_response (connection,
            200,
            response);

    ArguxRestResponse *resp = NULL;

    if (0 == strcmp(method, MHD_HTTP_METHOD_PUT)) {
        return MHD_NO;
    }

    if (0 == strcmp(method, MHD_HTTP_METHOD_GET)) {

        server->cb_read (url, &resp);

        return MHD_NO;
    }


    if (0 == strcmp(method, MHD_HTTP_METHOD_POST)) {
        //return MHD_queue_response (connection,
        //        400,
        //        bad_request_response);
        return MHD_NO;
    }

    if (0 == strcmp(method, MHD_HTTP_METHOD_DELETE)) {
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

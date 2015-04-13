/*-
 * Copyright (c) 2014 Stephan Arts. All Rights Reserved.
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

#include <time.h>

#include <curses.h>

#include <unistd.h>
#include <string.h>

#include <zmq.h>

#include "client.h"

static void *_ctx = NULL;
static void *_socket = NULL;
static char _uri[200];

static char _token[256];

#define RESPONSE_LEN 2048

static char response[RESPONSE_LEN];

int
client_connect_pass (
        const char *uri,
        const char *user,
        const char *password)
{
    char    msg[256];

    if (_ctx == NULL)
    {
        _ctx = zmq_ctx_new ();
    }

    if (_socket == NULL)
    {
        bzero (_token, 256);
        strncpy(_uri, uri, 200);
        _socket = zmq_socket (_ctx, ZMQ_REQ);
        zmq_connect (
                _socket,
                _uri);
        printf("SEND\n");
        zmq_send (
                _socket,
                "connect user/pass",
                17,
                0);
        printf("RECV\n");
        zmq_recv (
                _socket,
                msg,
                255,
                0);
        printf("%s\n", msg);
        sprintf(_token, "TOKEN_PLACEHOLDER");
        return 0;
    }

    return 1;
}

void
client_disconnect (
        void)
{
    zmq_disconnect(_socket, _uri);
    zmq_close(_socket);
    _socket = NULL;
}

int
client_connected (
        void)
{
    if (_socket == NULL) {
        return 0;
    }

    return 1;
}

void
client_send_cmd (
        const char *str,
        size_t len,
        char **resp,
        size_t *r_len)
{
    int l = 0;

    if (_socket == NULL) {
        return;
    }

    zmq_send (
        _socket,
        str,
        len,
        0);
    printf("RECV\n");
    l = zmq_recv (
        _socket,
        response,
        RESPONSE_LEN,
        0);

    *resp = response;
    *r_len = l;

    return;
}

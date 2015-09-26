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
#ifndef __REST_SERVER_H__
#define __REST_SERVER_H__

typedef struct _ArguxRestServer ArguxRestServer;

/** Response Object **/
typedef struct _ArguxRestResponse ArguxRestResponse;

struct _ArguxRestResponse
{
    int code;

    char *body;
    size_t body_len;
};

/** Callback Functions */

/**
 * \brief Callback for creating an object.
 *
 * Create object callback.
 *
 * \param url ..
 * \param upload_data ..
 * \param upload_data_size ..
 * \param resp ..
 */
typedef int (*ArguxCreateCallback)(
        const char *url,
        const char *upload_data,
        size_t upload_data_size,
        ArguxRestResponse **resp);

typedef int (*ArguxReadCallback)(
        const char *url,
        ArguxRestResponse **resp);

typedef int (*ArguxLookupPrincipalCallback)(
        const char *username,
        ArguxPrincipal **principal);


ArguxRestServer *
argux_rest_server_start();

void
argux_rest_server_stop(ArguxRestServer *);

int
argux_rest_server_set_create_cb (
        ArguxRestServer *server,
        ArguxCreateCallback cb_create);

int
argux_rest_server_set_read_cb (
        ArguxRestServer *server,
        ArguxReadCallback cb_read);

int
argux_rest_server_set_lookup_principal_cb (
        ArguxRestServer *server,
        ArguxLookupPrincipalCallback cb_lookup);

#endif

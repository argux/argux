/*-
 * Copyright (c) 2012 Stephan Arts. All Rights Reserved.
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

#if !defined (LIBARGUX_INSIDE_LIBARGUX_H) && !defined(LIBARGUX_COMPILATION)
#error "Only <libargux/libargux.h> can be included directly, "\
       "this file may disappear or change contents"
#endif

#ifndef __ARGUX_PLUGIN_DB_H__
#define __ARGUX_PLUGIN_DB_H__

typedef struct _ArguxPluginDB ArguxPluginDB;

struct _ArguxPluginDB
{
    ArguxPlugin plugin;

    char db_name[20];

    int schema_version;

    struct
    {
        int  (*setprop) (const char *key, const char *value);
        int  (*getprop) (const char *key, char **value);
        int  (*listprop)(char **keys[]);
        int  (*connect) (ArguxError **);
        int  (*disconnect) (ArguxError **);
        int  (*init) (ArguxError **);
    } db;

    /* Host */
    struct
    {
        int (*add)(
                const char *host,
                ArguxError **);
        int (*get)(
                const char *host,
                void **host_ptr,
                ArguxError **);
        int (*free)(
                const void *host_ptr,
                ArguxError **);
    } host;

    /* Namespace */
    struct
    {
        int (*add)(
                const char *ns,
                void *host_ptr,
                ArguxError **);
        int (*get)(
                const char *ns,
                void *host_ptr,
                void **ns_ptr,
                ArguxError **);
        int (*list)(
                char **nss,
                int *n_nss,
                void *host_ptr,
                ArguxError **);
        int (*free)(
                const void *ns_ptr,
                ArguxError **);
    } ns;

    /* Metric */
    struct
    {
        int (*add)(
                void *ns_ptr,
                const char *name,
                ArguxError **);
        int (*get)(
                const void *ns_ptr,
                const char *name,
                void **metric_ptr,
                ArguxError **);
        int (*copy)(
                const void *metric_ptr,
                void **metric_ptr_dst,
                ArguxError **);
        int (*free)(
                void **metric_ptr,
                ArguxError **);
    } metric;

    struct
    {
        int (*publish)(
                void *metric_ptr,
                const char *value,
                const time_t ts,
                ArguxError **);
    } value;


    /* Perm */
    struct 
    {
        struct
        {
            int (*set) (
                    const char *role,
                    const char *host,
                    unsigned int perm,
                    ArguxError **);
            int (*get) (
                    const char *role,
                    const char *host,
                    unsigned int *perm,
                    ArguxError **);
            int (*check) (
                    const char *role,
                    const char *host,
                    unsigned int perm,
                    ArguxError **);
        } host;
    } perm;

};


#endif                          /* __ARGUX_PLUGIN_DB_H__ */

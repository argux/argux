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

#include <openssl/sha.h>

#include <string.h>

#include <pcre.h>

#include <errno.h>

#include <sys/stat.h>

#include <sqlite3.h>

#include <libargux/libargux.h>

#include "plugin.h"
#include "db.h"
#include "host.h"
#include "ns.h"

#define HOSTNAME_MAXLEN 255
#define HOSTS_CACHESIZE 10

struct _Host
{
    int host_id;
    char name[HOSTNAME_MAXLEN+1];
};

Host _hosts_cache[HOSTS_CACHESIZE];
Host *_i_hosts_cache_name[HOSTS_CACHESIZE];
int  _n_hosts_cache = 0;

/**
 * _sqlite3_host_get
 *
 * @param name
 * @param host_ptr
 * @param error
 */
int
_sqlite3_host_get (
        const char *name,
        void **host_ptr,
        ArguxError **error)
{
    sqlite3 *db = _sqlite3_db_get();
    char query[128];
    sqlite3_stmt *handle = NULL;
    int ret;
    const char *errmsg;

    int host_id = -1;

    int min = 0;
    int mid = 0;
    int max = _n_hosts_cache-1;


    if (_n_hosts_cache > 0)
    {
        /* Find the host in the cache index */
        while(min < max)
        {
            mid = (max+min)/2;                

            if (strcmp(_i_hosts_cache_name[mid]->name, name) < 0)
            {
                min = mid+1;
            } else
            {
                max = mid;
            }
        }

        mid = (max+min)/2;                

        /* Check if the host is found */
        if ((max == min) &&
            (strcmp(_i_hosts_cache_name[mid]->name,
                    name) == 0))
        {
            argux_log_debug("CACHE HIT\n");

            (*host_ptr) = _i_hosts_cache_name[mid];

            return 0;
        }
    }

    argux_log_debug("CACHE MISS\n");

    sqlite3_snprintf(
            128,
            query,
            "SELECT id FROM 'HOSTS' WHERE name='%q';",
            name);

    ret = sqlite3_prepare_v2 (
            db,
            query,
            -1,
            &handle,
            NULL);
    if (ret != SQLITE_OK)
    {
        if (error != NULL) {
            errmsg = sqlite3_errmsg (db);
            *error = argux_error_new (
                    "%s",
                    errmsg);
        }
        return -1;
    }

    /* step until response != SQLITE_BUSY */
    do
    {
        ret = sqlite3_step (handle);
    } while (ret == SQLITE_BUSY);

    switch (ret)
    {
        case SQLITE_INTERRUPT:
        case SQLITE_SCHEMA:
        case SQLITE_CORRUPT:
        default: /* Error */
            if (error != NULL) {
                errmsg = sqlite3_errmsg (db);
                if (errmsg != NULL) {
                    *error = argux_error_new (
                            "%s",
                            errmsg);
                }
            }
            sqlite3_finalize(handle);
            return -1;
            break;
        case SQLITE_DONE:
            if (error != NULL) {
                *error = argux_error_new (
                        "Host '%s' not found.",
                        name);
            }
            sqlite3_finalize(handle);
            return -1;
            break;
        case SQLITE_ROW: /* Found a host */
            host_id = sqlite3_column_int (handle, 0);
            break;
    }

    /* Next step should finalize the request */
    ret = sqlite3_step (handle);
    switch (ret)
    {
        case SQLITE_INTERRUPT:
        case SQLITE_SCHEMA:
        case SQLITE_CORRUPT:
            if (error != NULL) {
                /* Something went wrong */
                errmsg = sqlite3_errmsg (db);
                *error = argux_error_new (
                        "%s",
                        errmsg);
            }
            sqlite3_finalize(handle);
            return -1;
            break;
        case SQLITE_ROW:
            /* Serious error, the UNIQUE constraint of the hosts
             * table is not honored (or missing).
             */
            if (error != NULL) {
                *error = argux_error_new (
                        "Multiple entries of host '%s'.\n",
                        name);
            }
            sqlite3_finalize(handle);
            return -1;
            break;
        case SQLITE_DONE:
            /* Done */
            sqlite3_finalize(handle);
            break;
    }


/****/

    void *buf = malloc (sizeof (void *)*_n_hosts_cache-mid);
    memcpy (buf, &_i_hosts_cache_name[mid], _n_hosts_cache-mid);
    _i_hosts_cache_name[mid] = &_hosts_cache[_n_hosts_cache];
    memcpy (&_i_hosts_cache_name[mid+1], buf, _n_hosts_cache-mid);


    (*host_ptr) = _i_hosts_cache_name[mid];
    strncpy(_i_hosts_cache_name[mid]->name, name, HOSTNAME_MAXLEN);
    _i_hosts_cache_name[mid]->host_id = host_id;

   _n_hosts_cache++;

    return 0;
}

/**
 * _sqlite3_host_add
 *
 * @param name
 * @param error
 */
int
_sqlite3_host_add (
        const char *host,
        ArguxError **error)
{
    char query[128];
    sqlite3_stmt *handle = NULL;
    int ret;
    const char *errmsg;
    void *host_ptr;

    if (_sqlite3_host_get (
            host,
            &host_ptr,
            NULL) == 0)
    {
        if (error != NULL) {
            *error = argux_error_new (
                    "host '%s' already exists.",
                    host);
        }
        return 1;
    }

    sqlite3 *db = _sqlite3_db_get();

    sqlite3_snprintf (
            128,
            query,
            "INSERT INTO 'HOSTS'(name) "
            "VALUES('%q');",
            host);

    ret = sqlite3_prepare_v2 (
            db,
            query,
            -1,
            &handle,
            NULL);
    if (ret != SQLITE_OK)
    {
        if (error != NULL) {
            errmsg = sqlite3_errmsg (db);
            *error = argux_error_new (
                    "%s",
                    errmsg);
        }
        return -1;
    }

    do
    {
        ret = sqlite3_step (handle);
    } while (ret == SQLITE_BUSY);

    switch (ret)
    {
        case SQLITE_DONE:
            break;
        case SQLITE_INTERRUPT:
        case SQLITE_SCHEMA:
        case SQLITE_CORRUPT:
        default:
            if (error != NULL) {
                errmsg = sqlite3_errmsg (db);
                *error = argux_error_new (
                        "%s",
                        errmsg);
            }
            sqlite3_finalize(handle);
            return -1;
            break;
    }

    sqlite3_finalize(handle);

    return 0;
}

/**
 * _sqlite3_host_getid
 *
 * @param host
 * @param error
 */
int
_sqlite3_host_getid (
        Host *host,
        ArguxError **error)
{
    return host->host_id;
}

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

#include <libargux/libargux.h>

#include "plugin.h"
#include "host.h"
#include "ns.h"
#include "db.h"
#include "metric.h"

ArguxPlugin *
argux_plugin_init ()
{
    const char *errors = NULL;
    int     err_offset;
    ArguxPluginDB *plugin;

    argux_log_info ("Initialise PostgreSQL plugin");

    plugin = argux_new (sizeof (ArguxPluginDB), 1);

    plugin->schema_version = 1;

    /* Copy the name */
    strncpy(plugin->db_name, "pgsql", 20);

    /* DB */
    plugin->db.setprop    = _pgsql_db_setprop;
    plugin->db.getprop    = _pgsql_db_getprop;
    plugin->db.listprop   = _pgsql_db_listprop;
    plugin->db.connect    = _pgsql_db_connect;
    plugin->db.disconnect = _pgsql_db_disconnect;
    plugin->db.init       = _pgsql_db_init;

    /* Host */
    plugin->host.get = _pgsql_host_get;
    plugin->host.add = _pgsql_host_add;

    /* Namespace */
    plugin->ns.add = _pgsql_ns_add;
    plugin->ns.get = _pgsql_ns_get;
    plugin->ns.list = _pgsql_ns_list;

    /* Metrics */
    plugin->metric.add = _pgsql_metric_add;
    plugin->metric.get = _pgsql_metric_get;
    plugin->metric.copy = _pgsql_metric_copy;
    plugin->metric.free = _pgsql_metric_free;

    /* Values */
    //plugin->value.publish = _pgsql_value_publish;

    return (ArguxPlugin *)plugin;
}

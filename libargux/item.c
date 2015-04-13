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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_SYSLOG_H
#include <sys/syslog.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <stdarg.h>

#include <arpa/inet.h>

#include <time.h>


#include "assert.h"
#include "error.h"
#include "log.h"
#include "types.h"
#include "itemtype.h"
#include "item.h"
#include "memory.h"

#define ARGUX_ITEM_NAME_SIZE 128

struct _ArguxItem
{
    char    name[ARGUX_ITEM_NAME_SIZE];

    ArguxItemType type;

    double  interval;
    int     active;
    int     remote;

    time_t  last_update;
};

static ArguxItem *_items = NULL;
static size_t _n_max_items = 0;
static size_t _n_items = 0;

static ArguxItem **_i_items_name = NULL;

static int
_item_name_compare (const void *p1, const void *p2)
{
    const ArguxItem *a = (const ArguxItem *)p1;
    const ArguxItem *b = (const ArguxItem *)p2;

    return strcmp (a->name, b->name);
}

static int
_item_name_find (const void *p1, const void *p2)
{
    const char *a = (const char *)p1;
    const ArguxItem *b = *(const ArguxItem **)p2;

    return strcmp (a, b->name);
}


void
argux_items_init (
        size_t n_max_items)
{
    if (_items != NULL)
    {
        argux_assert_critical (
                "items already initialized!");
        return;
    }
    _items = argux_new (
            sizeof (ArguxItem),
            n_max_items);

    _i_items_name = argux_new (
            sizeof (ArguxItem *),
            n_max_items);

    _n_max_items = n_max_items;
}

ArguxItem *
argux_item_register (
        const char *name,
        ArguxItemType type,
        double interval,
        int active,
        int remote,
        ArguxError **error)
{
    ArguxItem s_item;
    ArguxItem *item = NULL;

    strcpy (s_item.name, name);

    /**
     * Check if the item-registry has space for a new
     * entry.
     */
    if (_n_max_items == _n_items)
    {
        argux_log_error (
                "Max items reached (%d)",
                _n_max_items);
        return NULL;
    }
    item = &_items[_n_items];

    if (bsearch (
                    &s_item,
                    &_i_items_name,
                    _n_items,
                    sizeof (ArguxItem *),
                    _item_name_compare) != NULL)
    {
        argux_log_error (
                "Duplicate entry '%s'.",
                name);
        return NULL;
    }
    _i_items_name[_n_items] = item;

    qsort (
            &_i_items_name,
            _n_items + 1,
            sizeof (ArguxItem *),
            _item_name_compare);

    _n_items++;

    argux_log_debug ("New Item: %s", name);

    item->last_update = -1;

    return item;
}

void
argux_item_unregister (
        ArguxItem *item)
{

}

void
argux_item_get_interval (
        ArguxItem *item,
        double *interval)
{
    (*interval) = item->interval;
}

void
argux_item_set_interval (
        ArguxItem *item,
        double interval)
{
    item->interval = interval;
}

void
argux_item_get_last_update (
        ArguxItem *item,
        time_t *last_update)
{
    (*last_update) = item->last_update;
}

void
argux_item_set_last_update (
        ArguxItem *item,
        time_t last_update)
{
    item->last_update = last_update;
}

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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "memory.h"
#include "slist.h"

struct _ArguxSList
{
    void *data;

    ArguxSList *next;
};

ArguxSList *
argux_slist_new(void *data)
{
    ArguxSList *list = argux_new (sizeof(ArguxSList), 1);

    list->data = data;

    list->next = NULL;

    return list;
}

ArguxSList *
argux_slist_append(ArguxSList *list, void *data)
{
    ArguxSList *iter = list;
    ArguxSList *item = argux_new (sizeof(ArguxSList), 1);

    item->data = data;

    item->next = NULL;

    if (iter == NULL)
    {
        return item;
    }

    while (iter->next != NULL)
    {
        iter = iter->next;
    }

    iter->next = item;

    return list;
} 

ArguxSList *
argux_slist_prepend(ArguxSList *list, void *data)
{

    ArguxSList *item = argux_new (sizeof(ArguxSList), 1);

    item->data = data;

    item->next = list;

    return item;

}

ArguxSList *
argux_slist_next (ArguxSList *list)
{
    return list->next;
}

void *
argux_slist_get_data (ArguxSList *list)
{
    return list->data;
}

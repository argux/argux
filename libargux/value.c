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

#include <time.h>

#include "error.h"
#include "assert.h"
#include "log.h"
#include "memory.h"
#include "itemtype.h"
#include "item.h"
#include "value.h"

struct _ArguxValue
{
    ArguxItem *item;

    time_t  timestamp;

    union
    {
        char   *str;
        double  d;
        int     i;
    }       value;
};

ArguxValue *
argux_value_new (
        ArguxItem *item)
{
    if (item == NULL)
    {
        argux_assert_critical ("item = NULL");
    }
    ArguxValue *value = argux_new (sizeof (ArguxValue), 1);

    value->item = item;

    return value;
}

void
argux_value_set_string (
        ArguxValue *val,
        char *str,
        time_t time)
{
    val->value.str = str;
    val->timestamp = time;
}

void
argux_value_set_int (
        ArguxValue *val,
        int i,
        time_t time)
{
    val->value.i = i;
    val->timestamp = time;
}

void
argux_value_set_double (
        ArguxValue *val,
        double d,
        time_t time)
{
    val->value.d = d;
    val->timestamp = time;
}

char   *
argux_value_get_string (
        ArguxValue *val)
{
    return val->value.str;
}

int
argux_value_get_int (
        ArguxValue *val)
{
    return val->value.i;
}

double
argux_value_get_double (
        ArguxValue *val)
{
    return val->value.d;
}

time_t
argux_value_get_time (
        ArguxValue *val)
{
    return val->timestamp;
}

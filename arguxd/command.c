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
 * LOSS OF USE,DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
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

#include <string.h>

#include <pcre.h>

#include <libargux/libargux.h>


#include "command.h"

#define OVECCOUNT 30

ArguxSList *list = NULL;

typedef struct _Command Command;

struct _Command
{
    char name[255];

    int (*cmd)(char **args, int argc);
};


int
parse_command (const char *cmd, int len, char ***args, int *argc)
{
    const char *r_error;
    int r_error_offset;
    pcre *re;
    int rc;
    int r_ovector[OVECCOUNT];
    int i = 0;

    re = pcre_compile (
        "^(?P<command>[a-zA-Z\\-]+)(?:[ ]([^ ]+))*",
        0,
        &r_error,
        &r_error_offset,
        NULL);

    if (re == NULL) {
        printf("PCRE compilation failed at offset %d: %s\n", r_error_offset, r_error);
        return 1;
    }

    rc = pcre_exec (
        re,
        NULL,
        cmd,
        len,
        0,
        0,
        r_ovector,
        OVECCOUNT);

    if (rc < 0)
    {
        switch(rc)
        {
            case PCRE_ERROR_NOMATCH: printf("No match\n"); break;
            /*
            Handle other special cases if you like
            */
            default: printf("Matching error %d\n", rc); break;
        }
        pcre_free(re);     /* Release memory used for the compiled pattern */
        return 1;
    }

    *argc = rc;
    *args = calloc(rc+1, sizeof(char *));
    for (i = 1; i < rc; ++i) {
        pcre_get_substring(cmd, r_ovector, rc, i, (const char **)&(*args)[i-1]);
    }

    pcre_free(re);     /* Release memory used for the compiled pattern */
    return 0;
}

int
register_command (const char *name, int (*cmd)(char **args, int argc))
{
    Command *command = argux_new(1, sizeof(Command));

    command->cmd = cmd;
    strncpy(command->name, name, 255);

    //argux_list_prepend (list, name, strlen(name), cmd);
    list = argux_slist_prepend(list, command);
    return 0;
}

int
run_command (const char *name, char **args, int argc)
{
    printf("AAAAAAAAAAAA(RUN)\n");
    return 0;
}

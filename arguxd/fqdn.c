/*-
 * Copyright (c) 2012-2014 Stephan Arts. All Rights Reserved.
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

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <signal.h>

#include <pwd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/un.h>

#include <netdb.h>


#include <dirent.h>

#include <pthread.h>

#include <limits.h>

#include <unistd.h>
#include <string.h>

static char _hostname[_POSIX_HOST_NAME_MAX+1];

char *
get_fqdn() {
    struct addrinfo hints, *info, *p;
    int gai_result;

    char hostname[_POSIX_HOST_NAME_MAX+1];
    hostname[_POSIX_HOST_NAME_MAX] = '\0';
    gethostname(hostname, _POSIX_HOST_NAME_MAX);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    strncpy(_hostname, hostname, _POSIX_HOST_NAME_MAX);

    if ((gai_result = getaddrinfo(hostname, NULL, &hints, &info)) != 0) {
        return _hostname;
    }

    for(p = info; p != NULL; p = p->ai_next) {
        if(p->ai_canonname != NULL) {
            /* Do not return localhost */
            if (strcmp(p->ai_canonname, "localhost")) {
                printf("found: %s", p->ai_canonname);
                strncpy(_hostname, p->ai_canonname, _POSIX_HOST_NAME_MAX);
            }
        }
    }

    freeaddrinfo(info);

    return _hostname;
}

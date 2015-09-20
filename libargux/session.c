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

#include <stdio.h>

#include <string.h>

#include <openssl/sha.h>

#include "session.h"

#define DATA_LEN 128

/**
 * Warning:
 * Critical security issue. Random Seed source must be configurable,
 * this code (and the rest-server) MUST move to arguxd instead
 * of this library.
 *
 * While the rest of the code that depends on this function to
 * deliver a unique id is being written, we'll use a predictable
 * seed. this integer.
 *
 * NOTE TO SELF:
 * This is how security-problems get introduced.
 */
static int _session_idx = 0;

int
argux_sessionid_generate (char *buffer)
{
    int i;
    char data[DATA_LEN];
    unsigned char buf[SHA256_DIGEST_LENGTH];

    SHA256_CTX ctx;
    SHA256_Init (&ctx); 

    //SHA256_Update (&ctx, data, DATA_LEN);
    SHA256_Update (&ctx, &_session_idx, sizeof(int));

    _session_idx++;

    SHA256_Final (buf, &ctx);

    for (i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        sprintf(&buffer[i*2], "%02x", (unsigned int)buf[i]);
    }

    return 0;
}

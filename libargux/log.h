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
#error "Only <libargux/libargux.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __ARGUX_LOG_H__
#define __ARGUX_LOG_H__

#define ARGUX_LOG_WARNING   4
#define ARGUX_LOG_NOTICE    5
#define ARGUX_LOG_INFO      6
#define ARGUX_LOG_DEBUG     7

void
        argux_log_init (int);

void
        argux_log_mask (int);

void
        argux_log_alert (const char *format,...);

void
        argux_log_critical (const char *format,...);

void
        argux_log_error (const char *format,...);

void
        argux_log_warning (const char *format,...);

void
        argux_log_notice (const char *format,...);

void
        argux_log_info (const char *format,...);

void
        argux_log_debug (const char *format,...);
#endif

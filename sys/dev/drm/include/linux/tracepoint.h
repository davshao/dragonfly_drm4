/* Public domain. */

/*
 * Copyright (c) 2019 François Tigeot <ftigeot@wolfpond.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LINUX_TRACEPOINT_H
#define _LINUX_TRACEPOINT_H

// #include <linux/smp.h>
// #include <linux/errno.h>
#include <linux/types.h>
// #include <linux/rcupdate.h>

// #define EXPORT_TRACEPOINT_SYMBOL(name)

#define TP_PROTO(x...) x
 
#define DEFINE_EVENT(template, name, proto, args) \
static inline void trace_##name(proto) {} \
static inline bool trace_##name##_enabled(void) { return false; }

#define DEFINE_EVENT_PRINT(template, name, proto, args, print) \
static inline void trace_##name(proto) {}

#define TRACE_EVENT(name, proto, args, tstruct, assign, print) \
static inline void trace_##name(proto) {} \
static inline bool trace_##name##_enabled(void) { return false; }

#define TRACE_EVENT_CONDITION(name, proto, args, cond, tstruct, assign, print) \
static inline void trace_##name(proto) {}

#define DECLARE_EVENT_CLASS(name, proto, args, tstruct, assign, print) \
static inline void trace_##name(proto) {}

#endif

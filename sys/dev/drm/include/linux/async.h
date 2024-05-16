/* Public domain. */

/*
 * Copyright (c) 2015-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_ASYNC_H
#define _LINUX_ASYNC_H

#include <linux/types.h>
#include <linux/list.h>

typedef uint64_t async_cookie_t;
typedef void (*async_func_t) (void *, async_cookie_t);

#if defined(__OpenBSD__)
static inline async_cookie_t
async_schedule(async_func_t func, void *data)
{
	func(data, 0);
	return 0;
}
#else
/* DragonFly uses a static lock to synchronize */
extern async_cookie_t
async_schedule(async_func_t func, void *data);
#endif

static inline void async_synchronize_full(void) {
	/*
	 * XXX: This function is supposed to wait until all asynchronous
	 * function calls have been done
	 */
}

/*
 * This function is supposed to wait until all asynchronous function calls
 * prior to cookie have finished
 *
 */
static inline void
async_synchronize_cookie(async_cookie_t cookie)
{
}

static inline bool
current_is_async(void)
{
	/* Linux async functions are executed synchronously on DragonFly */
	return false;
}

#endif

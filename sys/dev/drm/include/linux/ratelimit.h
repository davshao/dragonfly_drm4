/* Public domain. */

/*
 * Copyright (c) 2018 François Tigeot <ftigeot@wolfpond.org>
 * Copyright (c) 2019 Jonathan Gray <jsg@openbsd.org>
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

#ifndef _LINUX_RATELIMIT_H
#define _LINUX_RATELIMIT_H

// #include <linux/spinlock.h>

struct ratelimit_state {
};

#if defined(__OpenBSD__) 
#define DEFINE_RATELIMIT_STATE(name, interval, burst) \
	struct ratelimit_state name
#else
#define DEFINE_RATELIMIT_STATE(name, interval_init, burst_init)		\
	int name __used = 1;
#endif

#define RATELIMIT_MSG_ON_RELEASE	(1 << 0)

#if defined(__OpenBSD__) 
static inline int
__ratelimit(struct ratelimit_state *rs)
{
	return 1;
}
#else
#define __ratelimit(x)	(1)
#endif

static inline void
ratelimit_state_init(struct ratelimit_state *rs, int interval, int burst)
{
}

static inline void
ratelimit_set_flags(struct ratelimit_state *rs, unsigned long flags)
{
}

#endif

/* Public domain. */

/* Then copied definition of USEC_TO_TIMEVAL */

/*
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)time.h	8.2 (Berkeley) 7/10/94
 */

#ifndef _LINUX_IOPOLL_H
#define _LINUX_IOPOLL_H

#if !defined(__OpenBSD__)
#include <sys/systm.h> /* DELAY() declared on DragonFly */
#include <sys/time.h>

static inline void
USEC_TO_TIMEVAL(uint64_t us, struct timeval *tv)
{
        tv->tv_sec = us / 1000000;
        tv->tv_usec = us % 1000000;
}
#endif

/* delay() in OpenBSD seems to correspond to DELAY in DragonFly */

#define readx_poll_timeout(op, addr, val, cond, sleep_us, timeout_us)	\
({									\
	struct timeval __end, __now, __timeout_tv;			\
	int __timed_out = 0;						\
									\
	if (timeout_us) {						\
		microuptime(&__now);					\
		USEC_TO_TIMEVAL(timeout_us, &__timeout_tv);		\
		timeradd(&__now, &__timeout_tv, &__end);		\
	}								\
									\
	for (;;) {							\
		(val) = (op)(addr);					\
		if (cond)						\
			break;						\
		if (timeout_us) {					\
			microuptime(&__now);				\
			if (timercmp(&__end, &__now, <=)) {		\
				__timed_out = 1;			\
				break;					\
			}						\
		}							\
		if (sleep_us)						\
			DELAY((sleep_us) / 2);				\
	}								\
	(__timed_out) ? -ETIMEDOUT : 0;					\
})

#endif

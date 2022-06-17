/*	$OpenBSD: completion.h,v 1.9 2020/06/22 14:19:35 jsg Exp $	*/
/*
 * Copyright (c) 2015, 2018 Mark Kettenis
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copyright (c) 2014-2020 François Tigeot <ftigeot@wolfpond.org>
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
#ifndef _LINUX_COMPLETION_H
#define _LINUX_COMPLETION_H

#if defined(__OpenBSD__)
#include <sys/param.h>
#else
#include <sys/kernel.h>
#endif
#include <sys/systm.h>

#if defined(__OpenBSD__)
#include <sys/mutex.h>
#else
#include <sys/lock.h>
#endif
#include <linux/wait.h>
// #include <linux/errno.h> already in linux/wait.h

struct completion {
	u_int done;
#if defined(__OpenBSD__)
	struct mutex lock;
#else
	wait_queue_head_t wait;
#endif
};

static inline void
init_completion(struct completion *x)
{
	x->done = 0;
#if defined(__OpenBSD__)
	mtx_init(&x->lock, IPL_TTY);
#else
	init_waitqueue_head(&x->wait);
#endif
}

static inline void
reinit_completion(struct completion *x)
{
	x->done = 0;
}

static inline long
__wait_for_completion_generic(struct completion *x,
			      unsigned long timo, int flags)
{
	int start_jiffies, elapsed_jiffies, remaining_jiffies;
	bool timeout_expired = false, awakened = false;
	long ret = 1;

	start_jiffies = ticks;

	lockmgr(&x->wait.lock, LK_EXCLUSIVE);
	while (x->done == 0 && !timeout_expired) {
		ret = lksleep(&x->wait, &x->wait.lock, flags, "lwfcg", timo);
		switch(ret) {
		case EWOULDBLOCK:
			timeout_expired = true;
			ret = 0;
			break;
		case ERESTART:
			ret = -ERESTARTSYS;
			break;
		case 0:
			awakened = true;
			break;
		}
	}
	lockmgr(&x->wait.lock, LK_RELEASE);

	if (awakened) {
		elapsed_jiffies = ticks - start_jiffies;
		remaining_jiffies = timo - elapsed_jiffies;
		if (remaining_jiffies > 0)
			ret = remaining_jiffies;
	}

	return ret;
}

static inline u_long
wait_for_completion_timeout(struct completion *x, u_long timo)
{
#if defined(__OpenBSD__)
	int ret;

	KASSERT(!cold);

	mtx_enter(&x->lock);
	while (x->done == 0) {
		ret = msleep(x, &x->lock, 0, "wfct", timo);
		if (ret) {
			mtx_leave(&x->lock);
			/* timeout */
			return 0;
		}
	}
	if (x->done != UINT_MAX)
		x->done--;
	mtx_leave(&x->lock);

	return 1;
#else
	return __wait_for_completion_generic(x, timo, 0);
#endif
}

#ifndef MAX_SCHEDULE_TIMEOUT
#define MAX_SCHEDULE_TIMEOUT    (LONG_MAX)
#endif

#if 0
void wait_for_completion(struct completion *c);
#endif

static inline void
wait_for_completion(struct completion *x)
{
#if defined(__OpenBSD__)
	KASSERT(!cold);

	mtx_enter(&x->lock);
	while (x->done == 0) {
		msleep_nsec(x, &x->lock, 0, "wfcom", INFSLP);
	}
	if (x->done != UINT_MAX)
		x->done--;
	mtx_leave(&x->lock);
#else
	__wait_for_completion_generic(x, MAX_SCHEDULE_TIMEOUT, 0);
#endif
}

#if 0
int wait_for_completion_interruptible(struct completion *c);
#endif

/* This function can only return 0 or -ERESTARTSYS */
#if defined(__OpenBSD__)
static inline u_long
wait_for_completion_interruptible(struct completion *x)
{
	int ret;

	KASSERT(!cold);

	mtx_enter(&x->lock);
	while (x->done == 0) {
		ret = msleep_nsec(x, &x->lock, PCATCH, "wfci", INFSLP);
		if (ret) {
			mtx_leave(&x->lock);
			if (ret == EWOULDBLOCK)
				return 0;
			return -ERESTARTSYS;
		}
	}
	if (x->done != UINT_MAX)
		x->done--;
	mtx_leave(&x->lock);

	return 0;
}
#else
static inline int
wait_for_completion_interruptible(struct completion *x)
{
	long rv = __wait_for_completion_generic(x, MAX_SCHEDULE_TIMEOUT, PCATCH);

	if (rv == -ERESTARTSYS)
		return -ERESTARTSYS;

	return 0;
}
#endif

#if defined(__OpenBSD__)
static inline u_long
wait_for_completion_interruptible_timeout(struct completion *x, u_long timo)
{
	int ret;

	KASSERT(!cold);

	mtx_enter(&x->lock);
	while (x->done == 0) {
		ret = msleep(x, &x->lock, PCATCH, "wfcit", timo);
		if (ret) {
			mtx_leave(&x->lock);
			if (ret == EWOULDBLOCK)
				return 0;
			return -ERESTARTSYS;
		}
	}
	if (x->done != UINT_MAX)
		x->done--;
	mtx_leave(&x->lock);

	return 1;
}
#else
static inline long
wait_for_completion_interruptible_timeout(struct completion *c, u_long timo)
{
	return __wait_for_completion_generic(c, timo, PCATCH);
}
#endif

#define	INIT_COMPLETION(c)	(c.done = 0)

/*
 * Completion interlock and wakeup.  Be careful not to execute the wakeup
 * from inside the spinlock as this can deadlock if the IPIQ fifo is full.
 * (also note that wakeup() is asynchronous anyway, so no point doing that).
 */
static inline void
complete(struct completion *x)
{
#if defined(__OpenBSD__)
	mtx_enter(&x->lock);
	if (x->done != UINT_MAX)
		x->done++;
	mtx_leave(&x->lock);
	wakeup_one(x);
#else
	lockmgr(&x->wait.lock, LK_EXCLUSIVE);
	if (x->done != UINT_MAX)
		x->done++;
	lockmgr(&x->wait.lock, LK_RELEASE);
	wakeup_one(&x->wait);
#endif
}

static inline void
complete_all(struct completion *x)
{
#if defined(__OpenBSD__)
	mtx_enter(&x->lock);
	x->done = UINT_MAX;
	mtx_leave(&x->lock);
	wakeup(x);
#else
	lockmgr(&x->wait.lock, LK_EXCLUSIVE);
	x->done = UINT_MAX;
	lockmgr(&x->wait.lock, LK_RELEASE);
	wakeup(&x->wait);
#endif
}

/*
 * try_wait_for_completion: try to decrement a completion without blocking
 * 			    its thread
 * return: false if the completion thread would need to be blocked/queued
 * 	   true if a non-blocking decrement was successful
 */
static inline bool
try_wait_for_completion(struct completion *x)
{
#if defined(__OpenBSD__)
	mtx_enter(&x->lock);
	if (x->done == 0) {
		mtx_leave(&x->lock);
		return false;
	}
	if (x->done != UINT_MAX)
		x->done--;
	mtx_leave(&x->lock);
	return true;
#else
	bool ret = false;

	/* we can't decrement x->done below 0 */
	if (READ_ONCE(x->done) == 0)
		return false;

	lockmgr(&x->wait.lock, LK_EXCLUSIVE);
	if (x->done > 0) {
		x->done--;
		ret = true;
	}
	lockmgr(&x->wait.lock, LK_RELEASE);

	return ret;
#endif
}

#endif

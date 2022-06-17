/* Public domain. */

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

#ifndef _LINUX_MUTEX_H
#define _LINUX_MUTEX_H

#include <sys/stdint.h>
#if defined(__OpenBSD__)
#include <sys/rwlock.h>
#else
#include <sys/lock.h>
#endif
// #include <asm/current.h>
#include <linux/list.h>
#include <linux/lockdep.h>
#include <linux/atomic.h>
// #include <asm/processor.h>

#if defined(__OpenBSD__)
#define DEFINE_MUTEX(x)		struct rwlock x = RWLOCK_INITIALIZER(#x)
#else
#define DEFINE_MUTEX(mutex)	\
	struct lock mutex;	\
	LOCK_SYSINIT(mutex, &mutex, "lmutex", LK_CANRECURSE)
#endif

#define mutex_lock_interruptible_nested(rwl, subc) \
					mutex_lock_interruptible(rwl)

#if defined(__OpenBSD__)

#define mutex_lock(rwl)			rw_enter_write(rwl)
#define mutex_lock_nest_lock(rwl, sub)	rw_enter_write(rwl)
#define mutex_lock_nested(rwl, sub)	rw_enter_write(rwl)
#define mutex_trylock(rwl)		(rw_enter(rwl, RW_WRITE | RW_NOSLEEP) == 0)
#define mutex_unlock(rwl)		rw_exit_write(rwl)
#define mutex_is_locked(rwl)		(rw_status(rwl) != 0)

#else

#define mutex_lock(rwl)			lockmgr(rwl, LK_EXCLUSIVE)
#define mutex_lock_next_lock(rwl, sub)	mutex_lock(rwl)
#define mutex_lock_nested(rwl, sub)	mutex_lock(rwl)
#define mutex_trylock(rwl)		lockmgr_try(rwl, LK_EXCLUSIVE)
#define mutex_unlock(rwl)		lockmgr(rwl, LK_RELEASE)
#define mutex_is_locked(rwl)		(lockinuse(rwl))

#endif


#if defined(__OpenBSD__)

#define mutex_destroy(rwl)

#else

/* FreeBSD current linux_mutex_destroy is more than one call */
static inline void
mutex_destroy(struct lock *mutex)
{
	lockuninit(mutex);
}

#endif

static inline int
mutex_lock_interruptible(struct lock *rwl)
{
	if (lockmgr(rwl, LK_EXCLUSIVE|LK_SLEEPFAIL|LK_PCATCH))
		return -EINTR;
	return 0;
}

enum mutex_trylock_recursive_result {
	MUTEX_TRYLOCK_FAILED,
	MUTEX_TRYLOCK_SUCCESS,
	MUTEX_TRYLOCK_RECURSIVE
};

static inline enum mutex_trylock_recursive_result
mutex_trylock_recursive(struct lock *rwl)
{
	if (lockowned(rwl))
		return MUTEX_TRYLOCK_RECURSIVE;
	if (mutex_trylock(rwl))
		return MUTEX_TRYLOCK_SUCCESS;
	return MUTEX_TRYLOCK_FAILED;
}

/*	$OpenBSD: drm_linux.c,v 1.94 2022/09/16 01:48:07 jsg Exp $	*/
/*
 * Copyright (c) 2013 Jonathan Gray <jsg@openbsd.org>
 * Copyright (c) 2015, 2016 Mark Kettenis <kettenis@openbsd.org>
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

static inline int
atomic_dec_and_mutex_lock(atomic_t *v, struct lock *lock)
{
	if (atomic_add_unless(v, -1, 1))
		return 0;

	mutex_lock(lock);
	if (atomic_dec_return(v) == 0)
		return 1;
	mutex_unlock(lock);
	return 0;
}

#endif

/*
 * Copyright (c) 2015 Michael Neumann <mneumann@ntecs.de>
 * All rights reserved.
 * Copyright (c) 2003-2011 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Michael Neumann <mneumann@ntecs.de> and
 *    Matthew Dillon <dillon@backplane.com>
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

#ifndef _LINUX_WW_MUTEX_H_
#define _LINUX_WW_MUTEX_H_

// #include <linux/mutex.h>

/*
 * A basic, unoptimized implementation of wound/wait mutexes for DragonFly
 * modelled after the Linux API [1].
 *
 * [1]: http://lxr.free-electrons.com/source/include/linux/ww_mutex.h
 */

// #include <sys/errno.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/param.h>
#if defined(__OpenBSD__)
#include <sys/mutex.h>
#else
#include <sys/lock.h>
#endif
#include <machine/atomic.h>
#include <linux/compiler.h>
#include <linux/mutex.h>
#include <asm/current.h>

struct ww_class {
	volatile u_long		stamp;
	const char		*name;
};

struct ww_acquire_ctx {
	u_long			stamp;
	struct ww_class		*ww_class;
	int			acquired;
	int			unused01;
};

struct ww_mutex {
	struct lock			base;
	volatile int			acquired;
	struct ww_acquire_ctx		*ctx;
	u_long				stamp;	/* heuristic */
	int				blocked;
	int				unused01;
};

#define DEFINE_WW_CLASS(classname)	\
	struct ww_class classname = {	\
		.stamp = 0,		\
		.name = #classname	\
	}

#define DEFINE_WD_CLASS(classname)	\
	struct ww_class classname = {	\
		.stamp = 0,		\
		.name = #classname	\
	}

static inline int
__ww_mutex_lock(struct ww_mutex *lock, struct ww_acquire_ctx *ctx, bool slow, bool intr);

#if 0
extern void ww_acquire_init(struct ww_acquire_ctx *ctx,
			struct ww_class *ww_class);
extern void ww_acquire_done(struct ww_acquire_ctx *ctx);
extern void ww_acquire_fini(struct ww_acquire_ctx *ctx);
extern void ww_mutex_init(struct ww_mutex *ww, struct ww_class *ww_class);
extern int ww_mutex_lock(struct ww_mutex *ww, struct ww_acquire_ctx *ctx);
extern int ww_mutex_lock_slow(struct ww_mutex *ww, struct ww_acquire_ctx *ctx);
extern int ww_mutex_lock_interruptible(struct ww_mutex *ww,
			struct ww_acquire_ctx *ctx);
extern int ww_mutex_lock_slow_interruptible(struct ww_mutex *ww,
			struct ww_acquire_ctx *ctx);
extern void ww_mutex_unlock(struct ww_mutex *ww);
extern void ww_mutex_destroy(struct ww_mutex *ww);
#endif

static inline void
ww_acquire_init(struct ww_acquire_ctx *ctx, struct ww_class *ww_class) {
#if defined(__OpenBSD__)
	ctx->stamp = __sync_fetch_and_add(&ww_class->stamp, 1);
	ctx->ww_class = ww_class;
#else
	ctx->stamp = atomic_fetchadd_long(&ww_class->stamp, 1);
	ctx->ww_class = ww_class;
	ctx->acquired = 0;
#endif
}

static inline void
ww_acquire_done(__unused struct ww_acquire_ctx *ctx) {
}

static inline void
ww_acquire_fini(__unused struct ww_acquire_ctx *ctx) {
}

static inline void
ww_mutex_init(struct ww_mutex *lock, struct ww_class *ww_class) {
#if defined(__OpenBSD__)
	mtx_init(&lock->base, IPL_NONE);
	lock->acquired = 0;
	lock->ctx = NULL;
	lock->owner = NULL;
#else
	lockinit(&lock->base, ww_class->name, 0, LK_CANRECURSE);
	lock->acquired = 0;
	lock->ctx = NULL;
	lock->stamp = 0xFFFFFFFFFFFFFFFFLU;
	lock->blocked = 0;
#endif
}

/*
 * Returns 1 if locked, 0 otherwise
 */
static inline bool
ww_mutex_is_locked(struct ww_mutex *lock)
{
#if defined(__OpenBSD__)
	bool res = false;
	mtx_enter(&lock->base);
	if (lock->acquired > 0) res = true;
	mtx_leave(&lock->base);
	return res;
#else
	return (lockstatus(&lock->base, NULL) != 0);
#endif
}

/*
 * Returns 1 on success, 0 if contended.
 *
 * This call has no context accounting.
 */
static inline int
ww_mutex_trylock(struct ww_mutex *lock)
{
#if defined(__OpenBSD__)
	int res = 0;

	mtx_enter(&lock->base);
	/*
	 * In case no one holds the ww_mutex yet, we acquire it.
	 */
	if (lock->acquired == 0) {
		KASSERT(lock->ctx == NULL);
		lock->acquired = 1;
		lock->owner = curproc;
		res = 1;
	}
	mtx_leave(&lock->base);
	return res;
#else
	return (lockmgr(&lock->base, LK_EXCLUSIVE|LK_NOWAIT) == 0);
#endif
}

/*
 * When `slow` is `true`, it will always block if the ww_mutex is contended.
 * It is assumed that the called will not hold any (ww_mutex) resources when
 * calling the slow path as this could lead to deadlocks.
 *
 * When `intr` is `true`, the ssleep will be interruptible.
 */
static inline int
__ww_mutex_lock(struct ww_mutex *lock, struct ww_acquire_ctx *ctx, bool slow, bool intr) {
#if defined(__OpenBSD__)
	int err;

	mtx_enter(&lock->base);
	for (;;) {
		/*
		 * In case no one holds the ww_mutex yet, we acquire it.
		 */
		if (lock->acquired == 0) {
			KASSERT(lock->ctx == NULL);
			lock->acquired = 1;
			lock->ctx = ctx;
			lock->owner = curproc;
			err = 0;
			break;
		}
		/*
		 * In case we already hold the return -EALREADY.
		 */
		else if (lock->owner == curproc) {
			err = -EALREADY;
			break;
		}
		/*
		 * This is the contention case where the ww_mutex is
		 * already held by another context.
		 */
		else {
			/*
			 * Three cases:
			 *
			 * - We are in the slow-path (first lock to obtain).
                         *
			 * - No context was specified. We assume a single
			 *   resource, so there is no danger of a deadlock.
                         *
			 * - An `older` process (`ctx`) tries to acquire a
			 *   lock already held by a `younger` process.
                         *   We put the `older` process to sleep until
                         *   the `younger` process gives up all it's
                         *   resources.
			 */
			if (slow || ctx == NULL ||
			    (lock->ctx && ctx->stamp < lock->ctx->stamp)) {
				KASSERT(!cold);
				int s = msleep_nsec(lock, &lock->base,
				    intr ? PCATCH : 0,
				    ctx ? ctx->ww_class->name : "ww_mutex_lock",
				    INFSLP);
				if (intr && (s == EINTR || s == ERESTART)) {
					// XXX: Should we handle ERESTART?
					err = -EINTR;
					break;
				}
			}
			/*
			 * If a `younger` process tries to acquire a lock
			 * already held by an `older` process, we `wound` it,
			 * i.e. we return -EDEADLK because there is a potential
			 * risk for a deadlock. The `younger` process then
			 * should give up all it's resources and try again to
			 * acquire the lock in question, this time in a
			 * blocking manner.
			 */
			else {
				err = -EDEADLK;
				break;
			}
		}

	} /* for */
	mtx_leave(&lock->base);
	return err;
#else /* _wwlock from linux_wwmutex.c */
	int flags = LK_EXCLUSIVE;
	int err;

	if (intr)
		flags |= LK_PCATCH;

	/*
	 * Normal mutex if ctx is NULL
	 */
	if (ctx == NULL) {
		err = lockmgr(&lock->base, flags);
		if (err)
			err = -EINTR;
		return err;
	}

	/*
	 * A normal blocking lock can be used when ctx->acquired is 0 (no
	 * prior locks are held).  If prior locks are held then we cannot
	 * block here.
	 *
	 * In the non-blocking case setup our tsleep interlock using
	 * lock->blocked first.
	 */
	for (;;) {
		if (ctx->acquired != 0) {
			atomic_swap_int(&lock->blocked, 1);
			flags |= LK_NOWAIT;
			tsleep_interlock(lock, (intr ? PCATCH : 0));
		}
		err = lockmgr(&lock->base, flags);
		if (err == 0) {
			lock->ctx = ctx;
			lock->stamp = ctx->stamp;
			++ctx->acquired;
			return 0;
		}

		/*
		 * EINTR or ERESTART returns -EINTR.  ENOLCK and EWOULDBLOCK
		 * cannot happen (LK_SLEEPFAIL not set, timeout is not set).
		 */
		if (err != EBUSY)
			return -EINTR;

		/*
		 * acquired can only be non-zero in this path.
		 * NOTE: lock->ctx is not MPSAFE.
		 * NOTE: lock->stamp is heuristical, a race is possible.
		 */
		KKASSERT(ctx->acquired > 0);

		/*
		 * Unwind if we aren't the oldest.
		 */
		if (ctx->stamp > lock->stamp)
			return -EDEADLK;

		/*
		 * We have priority over the currently held lock.  We have
		 * already setup the interlock so we can tsleep() until the
		 * remote wakes us up (which may have already happened).
		 *
		 * err is zero if woken up
		 *	    EINTR / ERESTART - signal
		 *	    EWOULDBLOCK	     - timeout expired (if not 0)
		 */
		if (flags & LK_NOWAIT) {
			err = tsleep(lock, PINTERLOCKED | (intr ? PCATCH : 0),
				       ctx->ww_class->name, 0);
			if (intr && (err == EINTR || err == ERESTART))
				return -EINTR;
			flags &= ~LK_NOWAIT;
		}
		/* retry */
	}
#endif
}

static inline int
ww_mutex_lock(struct ww_mutex *lock, struct ww_acquire_ctx *ctx) {
	return __ww_mutex_lock(lock, ctx, false, false);
}
	
static inline void
ww_mutex_lock_slow(struct ww_mutex *lock, struct ww_acquire_ctx *ctx) {
	(void) __ww_mutex_lock(lock, ctx, true, false);
}

static inline int
ww_mutex_lock_interruptible(struct ww_mutex *lock, struct ww_acquire_ctx *ctx) {
	return __ww_mutex_lock(lock, ctx, false, true);
}
	
static inline int __must_check
ww_mutex_lock_slow_interruptible(struct ww_mutex *lock, struct ww_acquire_ctx *ctx) {
	return __ww_mutex_lock(lock, ctx, true, true);
}

static inline void
ww_mutex_unlock(struct ww_mutex *lock) {
#if defined(__OpenBSD__)
	mtx_enter(&lock->base);
	KASSERT(lock->owner == curproc);
	KASSERT(lock->acquired == 1);

	lock->acquired = 0;
	lock->ctx = NULL;
	lock->owner = NULL;
	mtx_leave(&lock->base);
	wakeup(lock);
#else
	struct ww_acquire_ctx *ctx;

	ctx = lock->ctx;
	if (ctx) {
		KKASSERT(ctx->acquired > 0);
		--ctx->acquired;
		lock->ctx = NULL;
		lock->stamp = 0xFFFFFFFFFFFFFFFFLU;
	}
	lockmgr(&lock->base, LK_RELEASE);
	if (atomic_swap_int(&lock->blocked, 0))
		wakeup(lock);
#endif
}

static inline void
ww_mutex_destroy(struct ww_mutex *lock) {
#if defined(__OpenBSD__)
	KASSERT(lock->acquired == 0);
	KASSERT(lock->ctx == NULL);
	KASSERT(lock->owner == NULL);
#else
	lockuninit(&lock->base);
#endif
}

#endif	/* _LINUX_WW_MUTEX_H_ */

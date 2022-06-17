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

#ifndef _LINUX_SPINLOCK_H
#define _LINUX_SPINLOCK_H

#include <sys/spinlock2.h>
#include <sys/lock.h>
#include <sys/thread2.h> /* crit_enter() and crit_exit() */

#include <linux/kernel.h>
#include <linux/spinlock_types.h>
#include <linux/preempt.h>
#include <linux/bottom_half.h>
#include <linux/atomic.h>

#include <linux/irqflags.h>

// #include <linux/typecheck.h>
// #include <linux/linkage.h>
// #include <linux/compiler.h>
// #include <linux/thread_info.h>
// #include <linux/stringify.h>

// #include <asm/barrier.h>

// #include <linux/rwlock.h>

#if defined(__OpenBSD__)
#define spin_lock_irqsave(_mtxp, _flags) do {			\
		_flags = 0;					\
		mtx_enter(_mtxp);				\
	} while (0)
#else
#define spin_lock_irqsave(lock, flags)	\
({					\
	local_irq_save(flags);		\
	preempt_disable();		\
	lockmgr(lock, LK_EXCLUSIVE);	\
})
#endif

#if defined(__OpenBSD__)
#define spin_lock_irqsave_nested(_mtxp, _flags, _subclass) do {	\
		(void)(_subclass);				\
		_flags = 0;					\
		mtx_enter(_mtxp);				\
	} while (0)
#else
#define spin_lock_irqsave_nested(lock, flags, subclass)	\
	    spin_lock_irqsave(lock, flags)
#endif

#if defined(__OpenBSD__)
#define spin_unlock_irqrestore(_mtxp, _flags) do {		\
		(void)(_flags);					\
		mtx_leave(_mtxp);				\
	} while (0)
#else
static inline void
spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	lockmgr(lock, LK_RELEASE);
	local_irq_restore(flags);
	preempt_enable();
}
#endif

#if defined(__OpenBSD__)
#define spin_trylock_irqsave(_mtxp, _flags)			\
({								\
	(void)(_flags);						\
	mtx_enter_try(_mtxp) ? 1 : 0;				\
})
#else /* not sure */
#define spin_trylock_irqsave(_mtxp, _flags)			\
({								\
	int retspin;						\
 	local_irq_save(_flags);					\
	preempt_disable();					\
	retspin = lockmgr_try(_mtxp, LK_EXCLUSIVE);		\
	if (!retspin) {						\
		local_irq_restore(_flags)			\
		preempt_enable();				\
	}							\
	retspin;						\
})
#endif

#if defined(__OpenBSD__)
static inline int
atomic_dec_and_lock(volatile int *v, struct mutex *mtxp)
{
	if (*v != 1) {
		atomic_dec(v);
		return 0;
	}

	mtx_enter(mtxp);
	atomic_dec(v);
	return 1;
}
#else
static inline int
atomic_dec_and_lock(atomic_t *v, struct lock *mtxp)
{
	if (v->counter != 1) {
		atomic_dec(v);
		return 0;
	}

	lockmgr(mtxp, LK_EXCLUSIVE);
	atomic_dec(v);
	return 1;
}
#endif

#define atomic_dec_and_lock_irqsave(_a, _mtxp, _flags)		\
	atomic_dec_and_lock(_a, _mtxp)

#if defined(__OpenBSD__)
#define spin_lock(mtxp)			mtx_enter(mtxp)
#else
/* DragonFly already defined in sys/spinlock2.h */
#define drm_spin_lock(mtxp)		lockmgr(mtxp, LK_EXCLUSIVE)
#endif

#if defined(__OpenBSD__)
#define spin_lock_nested(mtxp, l)	mtx_enter(mtxp)
#else
/* __i915_active_fence_set() i915/i915_active.c lock of dma_fence */
#define spin_lock_nested(mtxp, l)	\
({					\
	(void)(l);			\
	lockmgr((mtxp), LK_EXCLUSIVE);	\
})
#endif

#if defined(__OpenBSD__)
#define spin_unlock(mtxp)		mtx_leave(mtxp)
#else
/* DragonFly already defined in sys/spinlock2.h */
#define drm_spin_unlock(mtxp)		lockmgr(mtxp, LK_RELEASE)
#endif

#if defined(__OpenBSD__)
#define spin_lock_irq(mtxp)		mtx_enter(mtxp)
#else
/*
 * The spin_lock_irq() family of functions stop hardware interrupts
 * from being delivered to the local CPU.
 */
static inline void spin_lock_irq(spinlock_t *lock)
{
	local_irq_disable();
	preempt_disable();
	lockmgr(lock, LK_EXCLUSIVE);
}
#endif

#if defined(__OpenBSD__)
#define spin_unlock_irq(mtxp)		mtx_leave(mtxp)
#else
static inline void spin_unlock_irq(spinlock_t *lock)
{
	lockmgr(lock, LK_RELEASE);
	local_irq_enable();
	preempt_enable();
}
#endif

#if defined(__OpenBSD__)
#define assert_spin_locked(mtxp)	MUTEX_ASSERT_LOCKED(mtxp)
#else
#define assert_spin_locked(mtxp)	KKASSERT(lockinuse(mtxp))
#endif

/* kernel drm mtxp below is for rwlock_t */
#if defined(__OpenBSD__)
#define read_lock(mtxp)			mtx_enter(mtxp)
#else
#define read_lock(mtxp)			lockmgr(mtxp, LK_SHARED)
#endif

#if defined(__OpenBSD__)
#define read_unlock(mtxp)		mtx_leave(mtxp)
#else
#define read_unlock(mtxp)		lockmgr(mtxp, LK_RELEASE)
#endif

#if defined(__OpenBSD__)
#define write_lock(mtxp)		mtx_enter(mtxp)
#else
#define write_lock(mtxp)		lockmgr(mtxp, LK_EXCLUSIVE)
#endif

#if defined(__OpenBSD__)
#define write_unlock(mtxp)		mtx_leave(mtxp)
#else
#define write_unlock(mtxp)		lockmgr(mtxp, LK_RELEASE)
#endif

#define spin_is_locked(x)	spin_held(x)

/*
  XXX: the spin_lock_bh() and spin_unlock_bh() functions are possibly incorrect
  XXX: see also in_interrupt()
*/
static inline void
spin_lock_bh(struct lock *lock)
{
	crit_enter();
	lockmgr(lock, LK_EXCLUSIVE);
}

static inline void
spin_unlock_bh(struct lock *lock)
{
	lockmgr(lock, LK_RELEASE);
	crit_exit();
}

#endif

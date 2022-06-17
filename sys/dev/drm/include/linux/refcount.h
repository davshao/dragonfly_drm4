/* Public domain. */

/*
 * Copyright (c) 2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_REFCOUNT_H
#define _LINUX_REFCOUNT_H

#include <sys/lock.h>
#include <machine/limits.h>

#include <sys/types.h>
#include <linux/atomic.h>
#if 0
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#endif

#if defined(__OpenBSD__)
typedef atomic_t refcount_t;
#else
typedef struct refcount_struct {
	atomic_t refs;
} refcount_t;
#endif


#if defined(__OpenBSD__)
static inline bool
refcount_dec_and_test(uint32_t *p)
{
	return atomic_dec_and_test(p);
}
#else
static inline bool
refcount_dec_and_test(refcount_t *p)
{
	return atomic_dec_and_test(&p->refs);
}
#endif

#if defined(__OpenBSD__)
static inline bool
refcount_inc_not_zero(uint32_t *p)
{
	return atomic_inc_not_zero(p);
}
#else
static inline bool
refcount_inc_not_zero(refcount_t *p)
{
	return atomic_inc_not_zero(&p->refs);
}
#endif

#if defined(__OpenBSD__)
static inline void
refcount_set(uint32_t *p, int v)
{
	atomic_set(p, v);
}
#else
static inline void
refcount_set(refcount_t *p, unsigned int v)
{
	atomic_set(&p->refs, v);
}
#endif

#if defined(__OpenBSD__)
static inline bool
refcount_dec_and_lock_irqsave(volatile int *v, struct mutex *lock,
    unsigned long *flags)
{
	if (atomic_add_unless(v, -1, 1))
		return false;

	mtx_enter(lock);
	if (atomic_dec_return(v) == 0)
		return true;
	mtx_leave(lock);
	return false;
}
#else
/* not sure, ported from OpenBSD */
static inline bool
refcount_dec_and_lock_irqsave(refcount_t *v, struct lock *lock,
    unsigned long *flags)
{
	if (atomic_add_unless(&v->refs, -1, 1))
		return false;

	lockmgr(lock, LK_EXCLUSIVE);
	if (atomic_dec_return(&v->refs) == 0)
		return true;
	lockmgr(lock, LK_RELEASE);
	return false;
}
#endif

#define REFCOUNT_SATURATED	(INT_MIN / 2)

static inline void
refcount_inc(refcount_t *r)
{
	int old_value = atomic_fetchadd_int(&r->refs.counter, 1);

	if (old_value <= 0)
		refcount_set(r, REFCOUNT_SATURATED);
}

#endif

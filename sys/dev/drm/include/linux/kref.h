/*	$OpenBSD: kref.h,v 1.4 2020/06/17 02:58:15 jsg Exp $	*/
/*
 * Copyright (c) 2015 Mark Kettenis
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

/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_KREF_H
#define _LINUX_KREF_H

#include <sys/types.h>
#if defined(__OpenBSD__)
#include <sys/rwlock.h>
#include <sys/atomic.h>
#else
#include <sys/lock.h>
#endif
#include <linux/atomic.h>
#include <linux/compiler.h>
#include <linux/refcount.h>
// #include <linux/spinlock.h>

struct kref {
#if defined(__OpenBSD__)
	uint32_t refcount;
#else
	refcount_t refcount;
#endif
};

static inline void
kref_init(struct kref *ref)
{
#if defined(__OpenBSD__)
	atomic_set(&ref->refcount, 1);
#else
	atomic_set(&ref->refcount.refs, 1);
#endif
}

static inline unsigned int
kref_read(const struct kref *ref)
{
	return atomic_read(&ref->refcount.refs);
}

static inline void
kref_get(struct kref *ref)
{
#if defined(__OpenBSD__)
	atomic_inc_int(&ref->refcount);
#else
	refcount_inc(&ref->refcount);
#endif
}

/*
 * kref_get_unless_zero: Increment refcount for object unless it is zero.
 */
static inline int
__must_check
kref_get_unless_zero(struct kref *ref)
{
#if defined(__OpenBSD__)
	if (ref->refcount != 0) {
		atomic_inc_int(&ref->refcount);
		return (1);
	} else {
		return (0);
	}
#else
	return atomic_add_unless(&ref->refcount.refs, 1, 0);
#endif
}

static inline int
kref_put(struct kref *ref, void (*release)(struct kref *ref))
{
#if defined(__OpenBSD__)
	if (atomic_dec_int_nv(&ref->refcount) == 0) {
		release(ref);
		return 1;
	}
#else
	if (atomic_dec_and_test(&ref->refcount.refs)) {
		release(ref);
		return 1;
	}
#endif
	return 0;
}

#if 0
static inline int
kref_sub(struct kref *kref, unsigned int count,
	     void (*rel)(struct kref *kref))
{
	if (refcount_release_n(&kref->refcount.counter, count)) {
		rel(kref);
		return 1;
	}
	return 0;
}
#endif


static inline int
kref_put_mutex(struct kref *kref, void (*release)(struct kref *kref),
    struct lock *lock)
{
	if (!atomic_add_unless(&kref->refcount.refs, -1, 1)) {
		lockmgr(lock, LK_EXCLUSIVE);
		if (likely(atomic_dec_and_test(&kref->refcount.refs))) {
			release(kref);
			return 1;
		}
		lockmgr(lock, LK_RELEASE);
		return 0;
	}

	return 0;
}

static inline int
kref_put_lock(struct kref *kref, void (*release)(struct kref *kref),
    struct lock *lock)
{
	if (!atomic_add_unless(&kref->refcount.refs, -1, 1)) {
		lockmgr(lock, LK_EXCLUSIVE);
		if (likely(atomic_dec_and_test(&kref->refcount.refs))) {
			release(kref);
			return 1;
		}
		lockmgr(lock, LK_RELEASE);
		return 0;
	}

	return 0;
}

#endif

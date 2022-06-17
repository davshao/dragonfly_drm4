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

#ifndef _LINUX_LOCKDEP_H
#define _LINUX_LOCKDEP_H

#include <sys/systm.h>
#include <sys/lock.h>

#include <linux/smp.h>
// #include <linux/linkage.h>
// #include <linux/list.h>

struct lock_class_key {
};

struct pin_cookie {
};

#if defined(__OpenBSD__)
#define might_lock(lock)
#else
#define might_lock(lock) do { } while (0)
#endif

#define might_lock_nested(lock, subc)

#if defined(__OpenBSD__)

#define lockdep_assert_held(lock)	do { (void)(lock); } while(0)
#define lockdep_assert_held_once(lock)	do { (void)(lock); } while(0)
#define lockdep_assert_once(lock)	do { (void)(lock); } while(0)

#else

static inline void
lockdep_assert_held(struct lock *l)
{
	KKASSERT(lockinuse(l));
}

/* Not sure */
#define lockdep_assert_held_once(lock)	lockdep_assert_held(lock)	
#define lockdep_assert_once(lock)	lockdep_assert_held(lock)	

#endif

#define lockdep_assert_none_held_once()	do {} while(0)
#define lock_acquire(lock, a, b, c, d, e, f)

#if defined(__OpenBSD__)

#define lock_release(lock, a)
#define lock_acquire_shared_recursive(lock, a, b, c, d)

#else

#define lock_release(lock, b, c)			do {} while (0)
#define lock_acquire_shared_recursive(lock, b, c, d, e)	do {} while (0)

#endif

#define lockdep_set_subclass(a, b)
#define lockdep_unpin_lock(a, b)
#define lockdep_set_class(a, b)
#define lockdep_init_map(a, b, c, d)
#define lockdep_set_class_and_name(a, b, c)
#define lockdep_is_held(lock)		0

#define mutex_acquire(a, b, c, d)
#define mutex_release(a, b)

#define SINGLE_DEPTH_NESTING		0

#define lockdep_pin_lock(lock)		\
({					\
	struct pin_cookie pc = {};	\
	pc;				\
})

#endif

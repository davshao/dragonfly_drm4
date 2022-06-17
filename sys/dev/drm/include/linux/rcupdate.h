/* Public domain. */

/*
 * Copyright (c) 2017-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef LINUX_RCUPDATE_H
#define LINUX_RCUPDATE_H

#if 0
#include <linux/types.h>
#include <linux/cache.h>
#include <linux/spinlock.h>
#include <linux/threads.h>
#endif
#include <linux/cpumask.h>
#if 0
#include <linux/seqlock.h>
#include <linux/lockdep.h>
#include <linux/completion.h>
#include <linux/bug.h>
#endif
#include <linux/compiler.h>
#include <linux/preempt.h>
// #include <linux/ktime.h>

// #include <asm/barrier.h>

// #include <linux/rcutree.h>

struct rcu_head {
};

#define __rcu

#if defined(__OpenBSD__)

#define rcu_dereference(p)	(p)
#define rcu_dereference_raw(p)	(p)
#define rcu_dereference_protected(p, c)	(p)

#else

#define rcu_dereference(p)					\
({								\
	typeof(*(p)) *__rcu_dereference_tmp = READ_ONCE(p);	\
	__rcu_dereference_tmp;					\
})

#define rcu_dereference_raw(p)			\
	((__typeof(*p) *)READ_ONCE(p))

#define rcu_dereference_protected(p, condition)	\
	((typeof(*p) *)(p))

#endif

#if defined(__OpenBSD__)
#define rcu_dereference_check(p, c)	(p)
#else
/* Not sure */
#define rcu_dereference_check(p, c)	rcu_dereference(p)
#endif

#if defined(__OpenBSD__)

#define rcu_access_pointer(p)	(p)
#define RCU_INIT_POINTER(p, v)		do { (p) = (v); } while(0)
#define rcu_assign_pointer(p, v)	do { (p) = (v); } while(0)

#else

#define rcu_access_pointer(p)	((typeof(*p) *)READ_ONCE(p))

#define RCU_INIT_POINTER(p, v)		\
do {					\
	(p) = (v);			\
} while (0)

#define rcu_assign_pointer(p, v)	\
do {					\
	cpu_mfence();			\
	WRITE_ONCE((p), (v));		\
} while (0)

#endif

#if defined(__OpenBSD__)

#define rcu_read_lock()
#define rcu_read_unlock()

#else

static inline void
rcu_read_lock(void)
{
	preempt_disable();
}

static inline void
rcu_read_unlock(void)
{
	preempt_enable();
}

#endif

#define rcu_pointer_handoff(p)	(p)
#define init_rcu_head(h)
#define destroy_rcu_head(h)

/* Not sure */
#define rcu_replace_pointer(rp, p, c)		\
({						\
	__typeof(rp) __r = rp;			\
	rp = p;					\
	__r;					\
})

#if defined(__OpenBSD__)
#define kfree_rcu(objp, name)	do { free((void *)objp, M_DRM, 0); } while(0)
#else
extern void __kfree_rcu(void *ptr);

#define kfree_rcu(ptr, rcu_head)	\
do {					\
	__kfree_rcu(ptr);		\
} while (0)
#endif

#if defined(__OpenBSD__)
#define rcu_barrier()		__asm volatile("" : : : "memory")
#else
static inline void rcu_barrier(void) {}
#endif

typedef void (*rcu_callback_t)(struct rcu_head *head);

#if defined(__OpenBSD__)
static inline void
call_rcu(struct rcu_head *head, void (*fn)(struct rcu_head *))
{
	fn(head);
}
#else
extern void call_rcu(struct rcu_head *head, void (*func)(struct rcu_head *));
#endif

#define synchronize_rcu()

#if defined(__OpenBSD__)
#define synchronize_rcu_expedited()
#else
static inline void
synchronize_rcu_expedited(void)
{
	cpu_mfence();
}
#endif

#define cond_synchronize_rcu(x)
#define get_state_synchronize_rcu()	0

#endif

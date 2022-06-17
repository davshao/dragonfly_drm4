/* $OpenBSD: atomic.h,v 1.19 2022/08/28 02:54:43 jsg Exp $ */
/**
 * \file drm_atomic.h
 * Atomic operations used in the DRM which may or may not be provided by the OS.
 * 
 * \author Eric Anholt <anholt@FreeBSD.org>
 */

/*-
 * Copyright 2004 Eric Anholt
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2017 Mellanox Technologies, Ltd.
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

#ifndef _DRM_LINUX_ATOMIC_H_
#define _DRM_LINUX_ATOMIC_H_

#include <sys/types.h>
#if defined(__OpenBSD__)
#include <sys/mutex.h>
#include <machine/intr.h>
#else
#include <sys/param.h> /* NBBY on DragonFly */
#include <machine/atomic.h>
#include <machine/cpufunc.h>
#endif
#include <linux/types.h>
#include <linux/compiler.h>	/* via x86/include/asm/atomic.h */

#include <asm/bitsperlong.h>

// #include <asm/atomic.h>

// #include <asm/cmpxchg.h>
// #include <asm/barrier.h>

#if defined(__OpenBSD__)
#define ATOMIC_INIT(x)		(x)
#else
#define ATOMIC_INIT(i)		{ (i) }
#endif

#if defined(__OpenBSD__)
#define atomic_set(p, v)	WRITE_ONCE(*(p), (v))
#else
static inline void
atomic_set(atomic_t *v, int i)
{
	atomic_store_rel_int(&v->counter, i);
}
#endif

#if defined(__OpenBSD__)
#define atomic_read(p)		READ_ONCE(*(p))
#else
static inline int
atomic_read(const atomic_t *v)
{
	return READ_ONCE(v->counter);
}
#endif

#if defined(__OpenBSD__)

#define atomic_inc(p)		__sync_fetch_and_add(p, 1)
#define atomic_dec(p)		__sync_fetch_and_sub(p, 1)

#else

static inline int
atomic_inc(atomic_t *v)
{
	return atomic_fetchadd_int(&v->counter, 1) + 1;
}

static inline int
atomic_dec(atomic_t *v)
{
	return atomic_fetchadd_int(&v->counter, -1) - 1;
}

#endif

#if defined(__OpenBSD__)
#define atomic_add(n, p)	__sync_fetch_and_add(p, n)
#define atomic_sub(n, p)	__sync_fetch_and_sub(p, n)
#else
#define atomic_add(n, p)	atomic_add_return((n), (p))
#define atomic_sub(n, p)	atomic_sub_return((n), (p))
#endif

#if defined(__OpenBSD__)
#define atomic_and(n, p)	__sync_fetch_and_and(p, n)
#define atomic_or(n, p)		atomic_setbits_int(p, n)
#else
/* atomic_and: atomically mask bits in a variable */
#define atomic_and(mask, addr)		\
	/* atomic *addr &= mask; */		\
	__asm __volatile("lock andl %0, %1"	\
		:				\
		: "r" (mask), "m" (*addr)	\
		: "memory");

/* atomic_or: atomically set bits in a variable */
#define atomic_or(mask, addr)		\
	/* atomic *addr |= mask; */		\
	__asm __volatile("lock orl %0, %1"	\
		:				\
		: "r" (mask), "m" (*addr)	\
		: "memory");
#endif

#if defined(__OpenBSD__)
#define atomic_add_return(n, p) __sync_add_and_fetch(p, n)
#define atomic_sub_return(n, p) __sync_sub_and_fetch(p, n)
#else
static inline int
atomic_add_return(int i, atomic_t *v)
{
	return i + atomic_fetchadd_int(&v->counter, i);
}

static inline int
atomic_sub_return(int i, atomic_t *v)
{
	return atomic_fetchadd_int(&v->counter, -i) - i;
}
#endif

#define atomic_sub_and_test(n, p)	(atomic_sub_return(n, p) == 0)
#define atomic_inc_return(v)	atomic_add_return(1, (v))
#define atomic_dec_return(v)	atomic_sub_return(1, (v))

#if defined(__OpenBSD__)
#define atomic_dec_and_test(v)	(atomic_dec_return(v) == 0)
#define atomic_inc_and_test(v)	(atomic_inc_return(v) == 0)
#else
#define atomic_dec_and_test(v)	(atomic_sub_return(1, (v)) == 0)
#define atomic_inc_and_test(v)	(atomic_add_return(1, (v)) == 0)
#endif

#if defined(__OpenBSD__)

#define atomic_cmpxchg(p, o, n)	__sync_val_compare_and_swap(p, o, n)
#else
static inline int atomic_cmpxchg(atomic_t *p, int o, int n)
{
	return atomic_cmpxchg_int(&p->counter, o, n);
}
#endif

#if defined(__OpenBSD__)
#define cmpxchg(p, o, n)	__sync_val_compare_and_swap(p, o, n)
#else
#define cmpxchg(ptr, old, new) ({					\
	__typeof(*(ptr)) __ret;						\
									\
	CTASSERT(sizeof(__ret) == 1 || sizeof(__ret) == 2 ||		\
	    sizeof(__ret) == 4 || sizeof(__ret) == 8);			\
									\
	__ret = (old);							\
	switch (sizeof(__ret)) {					\
	case 1:								\
		while (!atomic_fcmpset_8((volatile int8_t *)(ptr),	\
		    (int8_t *)&__ret, (u64)(new)) && __ret == (old))	\
			;						\
		break;							\
	case 2:								\
		while (!atomic_fcmpset_16((volatile int16_t *)(ptr),	\
		    (int16_t *)&__ret, (u64)(new)) && __ret == (old))	\
			;						\
		break;							\
	case 4:								\
		while (!atomic_fcmpset_32((volatile int32_t *)(ptr),	\
		    (int32_t *)&__ret, (u64)(new)) && __ret == (old))	\
			;						\
		break;							\
	case 8:								\
		while (!atomic_fcmpset_64((volatile int64_t *)(ptr),	\
		    (int64_t *)&__ret, (u64)(new)) && __ret == (old))	\
			;						\
		break;							\
	}								\
	__ret;								\
})

#define cmpxchg64(p, o, n)	__sync_val_compare_and_swap(p, o, n)

#endif

#if defined(__OpenBSD__)
#define atomic_set_release(p, v)	atomic_set((p), (v))
#else
static inline void
atomic_set_release(atomic_t *v, int i)
{
	atomic_store_rel_int(&v->counter, i);
}
#endif

#if defined(__OpenBSD__)
#define atomic_andnot(bits, p)		atomic_clearbits_int(p,bits)
#else
static inline void
atomic_andnot(int i, atomic_t *v)
{
	/* v->counter = v->counter & ~i; */
	atomic_clear_int(&v->counter, i);
}
#endif

#if defined(__OpenBSD__)
#define atomic_fetch_inc(p)		__sync_fetch_and_add(p, 1)
#else /* not sure */
#define atomic_fetch_inc(p)		atomic_inc(p)
#endif

#if defined(__OpenBSD__)
#define atomic_fetch_xor(n, p)		__sync_fetch_and_xor(p, n)
#else
/* Returns the old value of v->counter */
static inline int
atomic_fetch_xor(int i, atomic_t *v)
{
	int val = READ_ONCE(v->counter);

	while (atomic_cmpxchg_int(&v->counter, val, val ^ i) == 0) {
	}

	return val;
}
#endif

#if defined(__OpenBSD__) || defined(__DragonFly__)
#define try_cmpxchg(p, op, n)						\
({									\
	__typeof(p) __op = (__typeof((p)))(op);				\
	__typeof(*(p)) __o = *__op;					\
	__typeof(*(p)) __p = __sync_val_compare_and_swap((p), (__o), (n)); \
	if (__p != __o)							\
		*__op = __p;						\
	(__p == __o);							\
})
#else /* not sure */
static inline bool
try_cmpxchg(int *p, int *op, int n)
{
	int old = *op;
	int pr = atomic_cmpxchg_int((volatile unsigned int*)p, old, n);
	if (pr != old)
		*op = pr;
	return (pr == old);
}
#endif

static inline bool
#if defined(__OpenBSD__)
atomic_try_cmpxchg(volatile int *p, int *op, int n)
#else
atomic_try_cmpxchg(atomic_t *p, int *op, int n)
#endif
{
#if defined(__OpenBSD__)
	return try_cmpxchg(p, op, n);
#elif defined(__DragonFly__)
	return try_cmpxchg(&p->counter, op, n);
#else /* not sure */
	int old = *op;
	int pr = atomic_cmpxchg_int(&p->counter, old, n);
	if (pr != old)
		*op = pr;
	return (pr == old);
#endif
}

#if defined(__OpenBSD__)

static inline int
atomic_xchg(volatile int *v, int n)
{
	__sync_synchronize();
	return __sync_lock_test_and_set(v, n);
}

#define xchg(v, n)	__sync_lock_test_and_set(v, n)

#else

#define atomic_xchg(v, n)		atomic_swap_int(&((v)->counter), n)

#define xchg(ptr, value)				\
({							\
	__typeof(value) __ret = (value);		\
							\
	switch (sizeof(value)) {			\
	case 8:						\
		__asm __volatile("xchgq %0, %1"		\
		    : "+r" (__ret), "+m" (*(ptr))	\
		    : : "memory");			\
		break;					\
	case 4:						\
		__asm __volatile("xchgl %0, %1"		\
		    : "+r" (__ret), "+m" (*(ptr))	\
		    : : "memory");			\
		break;					\
	case 2:						\
		__asm __volatile("xchgw %0, %1"		\
		    : "+r" (__ret), "+m" (*(ptr))	\
		    : : "memory");			\
		break;					\
	case 1:						\
		__asm __volatile("xchgb %0, %1"		\
		    : "+r" (__ret), "+m" (*(ptr))	\
		    : : "memory");			\
		break;					\
	default:					\
		panic("xchg(): invalid size %ld\n", sizeof(value)); \
	}						\
							\
	__ret;						\
})

#endif

static inline int
#if defined(__OpenBSD__) /* incorrect API? */
atomic_add_unless(volatile int *v, int n, int u)
#else
atomic_add_unless(atomic_t *v, int n, int u)
#endif
{
#if defined(__OpenBSD__)
	int o;

	do {
		o = *v;
		if (o == u)
			return 0;
	} while (__sync_val_compare_and_swap(v, o, o +n) != o);

	return 1;
#else
        int c, old;
        c = atomic_read(v);
        for (;;) {
                if (unlikely(c == u))
                        break;
                old = atomic_cmpxchg_int(&v->counter, c, c + n);
                if (likely(old == c))
                        break;
                c = old;
        }
        return c != u;
#endif
}

#define atomic_inc_not_zero(v)	atomic_add_unless((v), 1, 0)

static inline int
#if defined(__OpenBSD__) /* incorrect API? */
atomic_dec_if_positive(volatile int *v)
#else
atomic_dec_if_positive(atomic_t *v)
#endif
{
	int r, o;

#if defined(__OpenBSD__)
	do {
		o = *v;
		r = o - 1;
		if (r < 0)
			break;
	} while (__sync_val_compare_and_swap(v, o, r) != o);
#else
	o = atomic_read(v);
	for (;;) {
		r = o - 1;
		if (unlikely(r < 0))
			break;
		if (likely(atomic_fcmpset_int(&v->counter, &o, r)))
			break;
	}
#endif

	return r;
}

#define atomic_long_read(p)	READ_ONCE(*(p))

/* 32 bit powerpc lacks 64 bit atomics */
#if defined(__OpenBSD__) && (!defined(__powerpc__) || defined(__powerpc64__))

typedef int64_t atomic64_t;

#define ATOMIC64_INIT(x)	(x)

#else

typedef struct {
	long counter;
} atomic64_t;

#define ATOMIC64_INIT(x)	{ (x) }

#endif

#if defined(__OpenBSD__)

#define atomic64_set(p, v)	WRITE_ONCE(*(p), (v))
#define atomic64_read(p)	READ_ONCE(*(p))

#else
static inline void
atomic64_set(atomic64_t *v, long i)
{
	atomic_store_rel_long(&v->counter, i);
}

static inline int64_t
atomic64_read(atomic64_t *v)
{
	return atomic_load_acq_long(&v->counter);
}
#endif

#if defined(__OpenBSD__) /* incorrect API? */
static inline int64_t
atomic64_xchg(volatile int64_t *v, int64_t n)
{
	__sync_synchronize();
	return __sync_lock_test_and_set(v, n);
}
#else
#define atomic64_xchg(v, n)		atomic_swap_long(&((v)->counter), n)
#endif

#if defined(__OpenBSD__)
static inline int64_t
atomic64_cmpxchg(volatile int64_t *v, int64_t o, int64_t n)
{
	return __sync_val_compare_and_swap(v, o, n);
}
#else
#define atomic64_cmpxchg(v, o, n)						\
	(atomic_cmpset_long((volatile uint64_t *)(v),(o),(n)) ? (o) : (0))
#endif

#if defined(__OpenBSD__)
#define atomic64_add(n, p)	__sync_fetch_and_add_8(p, n)
#define atomic64_sub(n, p)	__sync_fetch_and_sub_8(p, n)
#else
// #define	atomic64_add(n, p)	atomic_add_return_long((n), (p))
// #define	atomic64_sub(n, p)	atomic_sub_return_long((n), (p))
static inline int64_t
atomic64_add(int64_t n, atomic64_t *p)
{
	return n + atomic_fetchadd_long(&p->counter, n);
}

static inline int64_t
atomic64_sub(int64_t n, atomic64_t *p)
{
	return atomic_fetchadd_long(&p->counter, -n) - n;
}
#endif

#if defined(__OpenBSD__)
#define atomic64_inc(p)		__sync_fetch_and_add_8(p, 1)
#else
/* From DragonFly sys/cpu/x86_64/include/atomic.h */
static inline void
atomic64_inc(atomic64_t *p)
{
	atomic_fetchadd_64(&p->counter, 1);
}
#endif

#if defined(__OpenBSD__)
#define atomic64_add_return(n, p) __sync_add_and_fetch_8(p, n)
#else
static inline long
atomic64_add_return(int64_t n, atomic64_t *p)
{
	return n + atomic_fetchadd_64(&p->counter, n);
}
#endif

#define atomic64_inc_return(p)	__sync_add_and_fetch_8(p, 1)

/* 32 bit powerpc lacks 64 bit atomics */
#if defined(__OpenBSD__) && defined(__powerpc__) && !defined(__powerpc64__)

extern struct mutex atomic64_mtx;

typedef struct {
	volatile int64_t val;
} atomic64_t;

#define ATOMIC64_INIT(x)	{ (x) }

static inline void
atomic64_set(atomic64_t *v, int64_t i)
{
	mtx_enter(&atomic64_mtx);
	v->val = i;
	mtx_leave(&atomic64_mtx);
}

static inline int64_t
atomic64_read(atomic64_t *v)
{
	int64_t val;

	mtx_enter(&atomic64_mtx);
	val = v->val;
	mtx_leave(&atomic64_mtx);

	return val;
}

static inline int64_t
atomic64_xchg(atomic64_t *v, int64_t n)
{
	int64_t val;

	mtx_enter(&atomic64_mtx);
	val = v->val;
	v->val = n;
	mtx_leave(&atomic64_mtx);

	return val;
}

static inline void
atomic64_add(int i, atomic64_t *v)
{
	mtx_enter(&atomic64_mtx);
	v->val += i;
	mtx_leave(&atomic64_mtx);
}

#define atomic64_inc(p)		atomic64_add(p, 1)

static inline int64_t
atomic64_add_return(int i, atomic64_t *v)
{
	int64_t val;

	mtx_enter(&atomic64_mtx);
	val = v->val + i;
	v->val = val;
	mtx_leave(&atomic64_mtx);

	return val;
}

#define atomic64_inc_return(p)		atomic64_add_return(p, 1)

static inline void
atomic64_sub(int i, atomic64_t *v)
{
	mtx_enter(&atomic64_mtx);
	v->val -= i;
	mtx_leave(&atomic64_mtx);
}
#endif

#if defined(__OpenBSD__) && defined(__LP64__)
typedef int64_t atomic_long_t;
#else
typedef struct {
        volatile long counter;
} atomic_long_t;
#endif

#if defined(__OpenBSD__)
#define atomic_long_set(p, v)		atomic64_set(p, v)
#else
static inline void
atomic_long_set(atomic_long_t *v, long i)
{
        WRITE_ONCE(v->counter, i);
}
#endif

#if defined(__OpenBSD__)
#define atomic_long_xchg(v, n)		atomic64_xchg(v, n)
#else
static inline long
atomic_long_xchg(atomic_long_t *v, long n)
{
        return atomic_swap_long(&v->counter, n);
}
#endif

#if defined(__OpenBSD__)
#define atomic_long_cmpxchg(p, o, n)	atomic_cmpxchg(p, o, n)
#else
static inline long
atomic_long_cmpxchg(atomic_long_t *v, long old, long new)
{
        long ret = old;

        for (;;) {
                if (atomic_fcmpset_long(&v->counter, &ret, new))
                        break;
                if (ret != old)
                        break;
        }
        return (ret);
}
#endif

#define atomic_long_add(i, v)		atomic64_add(i, v)
#define atomic_long_sub(i, v)		atomic64_sub(i, v)

static inline int
atomic_long_add_unless(atomic64_t *v, long a, long u)
{
	long c = atomic64_read(v);

	for (;;) {
		if (unlikely(c == u))
			break;
		if (likely(atomic_fcmpset_long(&v->counter, &c, c + a)))
			break;
	}
	return (c != u);
}

#define atomic_long_inc_not_zero(v)	atomic_long_add_unless((v), 1, 0)

#if defined(__OpenBSD__) && !defined(__LP64__)
typedef int32_t atomic_long_t;
#define atomic_long_set(p, v)		atomic_set(p, v)
#define atomic_long_xchg(v, n)		atomic_xchg(v, n)
#define atomic_long_cmpxchg(p, o, n)	atomic_cmpxchg(p, o, n)
#define atomic_long_add(i, v)		atomic_add(i, v)
#define atomic_long_sub(i, v)		atomic_sub(i, v)
#endif

// #include <asm/atomic64_64.h>

// #include <asm/barrier.h>

#if 0
typedef struct {
	volatile u_int counter;
} atomic_t;
#endif

#define	atomic_add_negative(i, v)	(atomic_add_return((i), (v)) < 0)

#if 0
#define atomic_cmpset(p, o, n)		atomic_cmpset_32(&((p)->counter), o, n)
#endif

#if 0
static inline int64_t
atomic_add_return_long(int64_t i, atomic64_t *v)
{
	return i + atomic_fetchadd_long(&v->counter, i);
}

static inline int64_t
atomic_sub_return_long(int64_t i, atomic64_t *v)
{
	return atomic_fetchadd_long(&v->counter, -i) - i;
}
#endif

#if 0
/* atomic_clear_mask: atomically clears a variable from the bit set in mask */
#define atomic_clear_mask(mask, addr)		\
	/* atomic *addr &= ~mask; */		\
	__asm __volatile("lock andl %0, %1"	\
		:				\
		: "r" (~mask), "m" (*addr)	\
		: "memory");
#endif

// #define smp_mb__before_atomic()	cpu_ccfence()
// #define smp_mb__after_atomic()	cpu_ccfence()

#define cmpxchg_relaxed(...)	cmpxchg(__VA_ARGS__)

// #include <asm-generic/atomic-long.h>

/*	$OpenBSD: atomic.h,v 1.22 2022/08/29 02:01:18 jsg Exp $	*/
/*
 * The AMD64 architecture is rather strongly ordered.  When accessing
 * normal write-back cacheable memory, only reads may be reordered with
 * older writes to different locations.  There are a few instructions
 * (clfush, non-temporal move instructions) that obey weaker ordering
 * rules, but those instructions will only be used in (inline)
 * assembly code where we can add the necessary fence instructions
 * ourselves.
 */

#define __membar(_f) do { __asm volatile(_f ::: "memory"); } while (0)
#define membar_enter()		__membar("mfence")
#define membar_exit()		__membar("")
#define membar_producer()	__membar("")
#define membar_consumer()	__membar("")
#define membar_sync()		__membar("mfence")

#define membar_enter_after_atomic()	__membar("")
#define membar_exit_before_atomic()	__membar("")

#define	NBLONG	(NBBY * sizeof(long))

#if defined(__OpenBSD__) /* incorrect API? */
static inline atomic_t
test_and_set_bit(u_int b, volatile void *p)
#else
static inline long 
test_and_set_bit(long b, volatile unsigned long *p)
#endif
{
#if defined(__DragonFly__)
	long prev;

	p += b / (sizeof(long) * NBBY);
	b %= sizeof(long) * NBBY;
	b = 1L << b;
	do {
		prev = *(volatile long *)p;
	} while (atomic_cmpset_long(p, prev, prev | b) == 0);

	return !!(prev & b);
#else /* not sure adaptation to DragonFly */
	unsigned int m = 1 << (b & 0x1f);
#if defined(__OpenBSD__)
	unsigned int prev = __sync_fetch_and_or((volatile u_int *)p + (b >> 5), m);
#else
	volatile u_int *ptr = (volatile u_int *)p + (b >> 5);
	unsigned int prev;
	do {
		prev = *(volatile u_int *)ptr;
	} while (atomic_cmpset_int(ptr, prev, prev | m) == 0);
#endif
	return (prev & m) != 0;
#endif
}

#if defined(__DragonFly__)
#define	clear_bit(i, a)							\
    atomic_clear_long(&((volatile long *)(a))[(i)/NBLONG], 1LU << ((i) % NBLONG))
#else
static inline void
clear_bit(u_int b, volatile void *p)
{
#if defined(__OpenBSD__)
	atomic_clearbits_int(((volatile u_int *)p) + (b >> 5), 1 << (b & 0x1f));
#else
	atomic_clear_int(((volatile u_int *)p) + (b >> 5), 1 << (b & 0x1f));
#endif
}
#endif

static inline void
clear_bit_unlock(u_int b, volatile void *p)
{
	membar_enter();
	clear_bit(b, p);
}

#if defined(__DragonFly__)
#define	set_bit(i, a)							\
    atomic_set_long(&((volatile long *)(a))[(i)/NBLONG], 1LU << ((i) % NBLONG))
#else
static inline void
set_bit(u_int b, volatile void *p)
{
#if defined(__OpenBSD__)
	atomic_setbits_int(((volatile u_int *)p) + (b >> 5), 1 << (b & 0x1f));
#else /* not sure adaptation to DragonFly */
	atomic_set_int(((volatile u_int *)p) + (b >> 5), 1 << (b & 0x1f));
#endif
}
#endif

static inline void
#if defined(__OpenBSD__)
__clear_bit(u_int b, volatile void *p)
#else
__clear_bit(int b, volatile unsigned long *p)
#endif
{
#if defined(__OpenBSD__)
	volatile u_int *ptr = (volatile u_int *)p;
	ptr[b >> 5] &= ~(1 << (b & 0x1f));
#else
	*(p + (b / BITS_PER_LONG)) &= ~(1LU << (b % BITS_PER_LONG));
#endif
}

static inline void
#if defined(__OpenBSD__) 
__set_bit(u_int b, volatile void *p)
#else
__set_bit(int b, volatile unsigned long *p)
#endif
{
#if defined(__OpenBSD__) 
	volatile u_int *ptr = (volatile u_int *)p;
	ptr[b >> 5] |= (1 << (b & 0x1f));
#else
	*(p + (b / BITS_PER_LONG)) |= (1LU << (b % BITS_PER_LONG));
#endif
}

#if defined(__DragonFly__)
#define	test_bit(i, a)							\
    !!(atomic_load_acq_long(&((volatile long *)(a))[(i)/NBLONG]) &	\
			    (1LU << ((i) % NBLONG)))
#else
static inline int
#if defined(__OpenBSD__)
test_bit(u_int b, const volatile void *p)
#else
test_bit(u_int b, volatile void *p)
#endif
{
#if defined(__OpenBSD__)
	return !!(((volatile u_int *)p)[b >> 5] & (1 << (b & 0x1f)));
#else
	return !!(atomic_load_acq_int(&((volatile u_int *)p)[b >> 5]) & (1 << (b & 0x1f)));
#endif
}
#endif

#if defined(__OpenBSD__)
static inline int
__test_and_set_bit(u_int b, volatile void *p)
{
	unsigned int m = 1 << (b & 0x1f);
	volatile u_int *ptr = (volatile u_int *)p;
	unsigned int prev = ptr[b >> 5];
	ptr[b >> 5] |= m;
	
	return (prev & m) != 0;
}
#else
/* Like atomic_testandset_long() but not atomic */
static inline bool
__test_and_set_bit(long b, volatile unsigned long *p)
{
	bool previous_bit;

	__asm __volatile(
		"btsq	%2,%1 ;"
		"setc	%0 ;"
		: "=q" (previous_bit),	/* %0 */
		  "+m" (*p)		/* %1 */
		: "Jr" (b)		/* %2 */
		: "cc");

	return previous_bit;
}
#endif

#if defined(__OpenBSD__)
static inline int
test_and_clear_bit(u_int b, volatile void *p)
#else
static inline long
test_and_clear_bit(long b, long *p)
#endif
{
#if defined(__DragonFly__)
	long prev;

	p += b / (sizeof(long) * NBBY);
	b %= sizeof(long) * NBBY;
	b = 1L << b;
	do {
		prev = *(volatile long *)p;
	} while (atomic_cmpset_long(p, prev, prev & ~b) == 0);

	return !!(prev & b);
#else /* not sure adaptation for DragonFly */
	unsigned int m = 1 << (b & 0x1f);
#if defined(__OpenBSD__)
	unsigned int prev = __sync_fetch_and_and((volatile u_int *)p + (b >> 5), ~m);
#else
	volatile u_int *ptr = (volatile u_int *)p + (b >> 5);
	unsigned int prev;
	do {
		prev = *(volatile u_int *)ptr;
	} while (atomic_cmpset_int(ptr, prev, prev & ~m) == 0);
#endif
	return (prev & m) != 0;
#endif
}

#if defined(__OpenBSD__)
static inline int
__test_and_clear_bit(u_int b, volatile void *p)
{
	volatile u_int *ptr = (volatile u_int *)p;
	int rv = !!(ptr[b >> 5] & (1 << (b & 0x1f)));
	ptr[b >> 5] &= ~(1 << (b & 0x1f));
	return rv;
}
#else
static inline unsigned long
__test_and_clear_bit(unsigned int b, volatile unsigned long *p)
{
	const unsigned int units = (sizeof(*p) * NBBY);
	volatile unsigned long *const ptr = &p[b / units];
	const unsigned long mask = (1UL << (b % units));
	unsigned long v;

	v = *ptr;
	*ptr &= ~mask;

	return ((v & mask) != 0);
}
#endif

#define	BIT_MASK(n)		(~0UL >> (BITS_PER_LONG - (n)))

static inline int
__ffsl(long mask)
{
	return (ffsl(mask) - 1);
}

static inline int
__flsl(long mask)
{
	return (flsl(mask) - 1);
}

#if defined(__OpenBSD__)
static inline int
find_first_zero_bit(volatile void *p, int max)
{
	int b;
	volatile u_int *ptr = (volatile u_int *)p;

	for (b = 0; b < max; b += 32) {
		if (ptr[b >> 5] != ~0) {
			for (;;) {
				if ((ptr[b >> 5] & (1 << (b & 0x1f))) == 0)
					return b;
				b++;
			}
		}
	}
	return max;
}
#else
static inline unsigned long
find_first_zero_bit(unsigned long *addr, unsigned long size)
{
	long mask;
	int bit;

	for (bit = 0; size >= BITS_PER_LONG;
	    size -= BITS_PER_LONG, bit += BITS_PER_LONG, addr++) {
		if (~(*addr) == 0)
			continue;
		return (bit + __ffsl(~(*addr)));
	}
	if (size) {
		mask = ~(*addr) & BIT_MASK(size);
		if (mask)
			bit += __ffsl(mask);
		else
			bit += size;
	}
	return (bit);
}
#endif

#if defined(__OpenBSD__)
static inline int
find_next_zero_bit(volatile void *p, int max, int b)
{
	volatile u_int *ptr = (volatile u_int *)p;

	for (; b < max; b += 32) {
		if (ptr[b >> 5] != ~0) {
			for (;;) {
				if ((ptr[b >> 5] & (1 << (b & 0x1f))) == 0)
					return b;
				b++;
			}
		}
	}
	return max;
}
#else
static inline unsigned long
find_next_zero_bit(unsigned long *addr, unsigned long size,
    unsigned long offset)
{
	long mask;
	int offs;
	int bit;
	int pos;

	if (offset >= size)
		return (size);
	pos = offset / BITS_PER_LONG;
	offs = offset % BITS_PER_LONG;
	bit = BITS_PER_LONG * pos;
	addr += pos;
	if (offs) {
		mask = ~(*addr) & ~BIT_MASK(offs);
		if (mask)
			return (bit + __ffsl(mask));
		bit += BITS_PER_LONG;
		addr++;
	}
	for (size -= bit; size >= BITS_PER_LONG;
	    size -= BITS_PER_LONG, bit += BITS_PER_LONG, addr++) {
		if (~(*addr) == 0)
			continue;
		return (bit + __ffsl(~(*addr)));
	}
	if (size) {
		mask = ~(*addr) & BIT_MASK(size);
		if (mask)
			bit += __ffsl(mask);
		else
			bit += size;
	}
	return (bit);
}
#endif

#if defined(__OpenBSD__)
static inline int
find_first_bit(volatile void *p, int max)
{
	int b;
	volatile u_int *ptr = (volatile u_int *)p;

	for (b = 0; b < max; b += 32) {
		if (ptr[b >> 5] != 0) {
			for (;;) {
				if (ptr[b >> 5] & (1 << (b & 0x1f)))
					return b;
				b++;
			}
		}
	}
	return max;
}
#else
static inline unsigned long
find_first_bit(unsigned long *addr, unsigned long size)
{
	long mask;
	int bit;

	for (bit = 0; size >= BITS_PER_LONG;
	    size -= BITS_PER_LONG, bit += BITS_PER_LONG, addr++) {
		if (*addr == 0)
			continue;
		return (bit + __ffsl(*addr));
	}
	if (size) {
		mask = (*addr) & BIT_MASK(size);
		if (mask)
			bit += __ffsl(mask);
		else
			bit += size;
	}
	return (bit);
}
#endif

#if defined(__OpenBSD__)
static inline int
find_next_bit(volatile void *p, int max, int b)
{
	volatile u_int *ptr = (volatile u_int *)p;

	for (; b < max; b+= 32) {
		if (ptr[b >> 5] != 0) {
			for (;;) {
				if (ptr[b >> 5] & (1 << (b & 0x1f)))
					return b;
				b++;
			}
		}
	}
	return max;
}
#else
static inline unsigned long
find_next_bit(unsigned long *addr, unsigned long size, unsigned long offset)
{
	long mask;
	int offs;
	int bit;
	int pos;

	if (offset >= size)
		return (size);
	pos = offset / BITS_PER_LONG;
	offs = offset % BITS_PER_LONG;
	bit = BITS_PER_LONG * pos;
	addr += pos;
	if (offs) {
		mask = (*addr) & ~BIT_MASK(offs);
		if (mask)
			return (bit + __ffsl(mask));
		bit += BITS_PER_LONG;
		addr++;
	}
	for (size -= bit; size >= BITS_PER_LONG;
	    size -= BITS_PER_LONG, bit += BITS_PER_LONG, addr++) {
		if (*addr == 0)
			continue;
		return (bit + __ffsl(*addr));
	}
	if (size) {
		mask = (*addr) & BIT_MASK(size);
		if (mask)
			bit += __ffsl(mask);
		else
			bit += size;
	}
	return (bit);
}
#endif

#if 0 /* unused in kernel drm */
static inline unsigned long
find_last_bit(unsigned long *addr, unsigned long size)
{
	long mask;
	int offs;
	int bit;
	int pos;

	pos = size / BITS_PER_LONG;
	offs = size % BITS_PER_LONG;
	bit = BITS_PER_LONG * pos;
	addr += pos;
	if (offs) {
		mask = (*addr) & BIT_MASK(offs);
		if (mask)
			return (bit + __flsl(mask));
	}
	while (--pos) {
		addr--;
		bit -= BITS_PER_LONG;
		if (*addr)
			return (bit + __flsl(mask));
	}
	return (size);
}
#endif

#define for_each_set_bit(b, p, max) \
	for ((b) = find_first_bit((p), (max));			\
	     (b) < (max);					\
	     (b) = find_next_bit((p), (max), (b) + 1))

#define for_each_clear_bit(b, p, max) \
	for ((b) = find_first_zero_bit((p), (max));		\
	     (b) < (max);					\
	     (b) = find_next_zero_bit((p), (max), (b) + 1))

#if defined(__OpenBSD__) && defined(__i386__)
#define rmb()	__asm volatile("lock; addl $0,-4(%%esp)" : : : "memory", "cc")
#define wmb()	__asm volatile("lock; addl $0,-4(%%esp)" : : : "memory", "cc")
#define mb()	__asm volatile("lock; addl $0,-4(%%esp)" : : : "memory", "cc")
#define smp_mb()	__asm volatile("lock; addl $0,-4(%%esp)" : : : "memory", "cc")
#define smp_rmb()	__membar("")
#define smp_wmb()	__membar("")
#define __smp_store_mb(var, value)	do { (void)xchg(&var, value); } while (0)
#define smp_mb__after_atomic()	do { } while (0)
#define smp_mb__before_atomic()	do { } while (0)
#endif

#if defined(__OpenBSD__) && defined(__amd64__)
#define rmb()	__membar("lfence")
#else
#define rmb()	cpu_lfence()
#endif

#if defined(__OpenBSD__) && defined(__amd64__)
#define wmb()	__membar("sfence")
#else
#define wmb()	cpu_sfence()
#endif

#if defined(__OpenBSD__) && defined(__amd64__)
#define mb()	__membar("mfence")
#else
#define mb()	cpu_mfence()
#endif

#if defined(__OpenBSD__) && defined(__amd64__)
#define smp_mb()	__asm volatile("lock; addl $0,-4(%%rsp)" : : : "memory", "cc")
#else
#define smp_mb()	mb()
#endif

#if defined(__OpenBSD__) && defined(__amd64__)
#define smp_rmb()	__membar("")
#else
#define smp_rmb()	rmb()
#endif

#if defined(__OpenBSD__) && defined(__amd64__)
#define smp_wmb()	__membar("")
#else
#define smp_wmb()	barrier()
#endif

#define __smp_store_mb(var, value)	do { (void)xchg(&var, value); } while (0)

#if defined(__OpenBSD__) && defined(__amd64__)
#define smp_mb__after_atomic()	do { } while (0)
#define smp_mb__before_atomic()	do { } while (0)
#else
#define smp_mb__after_atomic()	cpu_ccfence()
#define smp_mb__before_atomic()	cpu_ccfence()
#endif

#ifndef smp_rmb
#define smp_rmb()	rmb()
#endif

#ifndef smp_wmb
#define smp_wmb()	wmb()
#endif

#ifndef mmiowb
#if defined(__OpenBSD__)
#define mmiowb()	wmb()
#else
#define mmiowb()	cpu_sfence()
#endif
#endif

#ifndef smp_mb__before_atomic
#define smp_mb__before_atomic()	mb()
#endif

#ifndef smp_mb__after_atomic
#define smp_mb__after_atomic()	mb()
#endif

#ifndef smp_store_mb
#if defined(__OpenBSD__)
#define smp_store_mb(x, v)	do { x = v; mb(); } while (0)
#else
#define smp_store_mb(var, value) do {	\
	WRITE_ONCE(var, value);		\
	cpu_mfence();			\
} while (0)
#endif
#endif

#define smp_store_release(p, v)	do {	\
	cpu_ccfence();			\
	WRITE_ONCE(*p, v);		\
} while (0)

#endif

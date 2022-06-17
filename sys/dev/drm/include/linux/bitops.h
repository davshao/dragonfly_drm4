/*	$OpenBSD: bitops.h,v 1.4 2022/01/14 06:53:14 jsg Exp $	*/
/*
 * Copyright (c) 2013, 2014, 2015 Mark Kettenis
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
#ifndef _LINUX_BITOPS_H
#define _LINUX_BITOPS_H

#include <sys/types.h>
#include <sys/param.h>

#if defined(__DragonFly__)
#include <sys/systm.h>
#endif

#if defined(__OpenBSD__)
#include <lib/libkern/libkern.h>
#else
#include <machine/cpufunc.h>
#endif

#include <asm/bitsperlong.h>
#include <linux/atomic.h>

// #include <asm/types.h>

#define BIT(x)		(1UL << (x))
#define BIT_ULL(x)	(1ULL << (x))
#if defined(__OpenBSD__)
#define BIT_MASK(x)	(1UL << ((x) % BITS_PER_LONG))
#else
#if 0 /* previous DragonFly moved to linux/atomic.h */
#define	BIT_MASK(n)		(~0UL >> (BITS_PER_LONG - (n)))
#endif
#endif
#define BITS_PER_BYTE	8

/* Returns a contiguous bitmask from bits h to l */
#define GENMASK(h, l)		(((~0UL) >> (BITS_PER_LONG - (h) - 1)) & ((~0UL) << (l)))
#define GENMASK_ULL(h, l)	(((~0ULL) >> (BITS_PER_LONG_LONG - (h) - 1)) & ((~0ULL) << (l)))

#define BITS_PER_TYPE(x)	(8 * sizeof(x))
#define BITS_TO_LONGS(x)	howmany((x), 8 * sizeof(long))

#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)

// #include <asm/bitops.h>

#if defined(__OpenBSD__)
/* despite the name these are really ctz */
#define __ffs(x)		__builtin_ctzl(x)
#else
static inline int
__ffs(int mask)
{
	return (ffs(mask) - 1);
}
#endif
#define __ffs64(x)		__builtin_ctzll(x)

#if defined(__OpenBSD__)
static inline uint8_t
hweight8(uint32_t x)
{
	x = (x & 0x55) + ((x & 0xaa) >> 1);
	x = (x & 0x33) + ((x & 0xcc) >> 2);
	x = (x + (x >> 4)) & 0x0f;
	return (x);
}

static inline uint16_t
hweight16(uint32_t x)
{
	x = (x & 0x5555) + ((x & 0xaaaa) >> 1);
	x = (x & 0x3333) + ((x & 0xcccc) >> 2);
	x = (x + (x >> 4)) & 0x0f0f;
	x = (x + (x >> 8)) & 0x00ff;
	return (x);
}

static inline uint32_t
hweight32(uint32_t x)
{
	x = (x & 0x55555555) + ((x & 0xaaaaaaaa) >> 1);
	x = (x & 0x33333333) + ((x & 0xcccccccc) >> 2);
	x = (x + (x >> 4)) & 0x0f0f0f0f;
	x = (x + (x >> 8));
	x = (x + (x >> 16)) & 0x000000ff;
	return x;
}

static inline uint32_t
hweight64(uint64_t x)
{
	x = (x & 0x5555555555555555ULL) + ((x & 0xaaaaaaaaaaaaaaaaULL) >> 1);
	x = (x & 0x3333333333333333ULL) + ((x & 0xccccccccccccccccULL) >> 2);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
	x = (x + (x >> 8));
	x = (x + (x >> 16));
	x = (x + (x >> 32)) & 0x000000ff;
	return x;
}
#else /* bitcount16 to 64 in sys/system.h */
#define hweight8(x)	bitcount16((x) & 0xff)
#define hweight16(x)	bitcount16(x)
#define hweight32(x)	bitcount32(x)
#define hweight64(x)	bitcount64(x)
#endif

static inline unsigned long
hweight_long(unsigned long x)
{
#ifdef __LP64__
	return hweight64(x);
#else
	return hweight32(x);
#endif
}

static inline int64_t
sign_extend64(uint64_t value, int index)
{
	uint8_t shift = 63 - index;
	return (int64_t)(value << shift) >> shift;
}

#if defined(__OpenBSD__)
static inline int
fls64(long long mask)
{
	int bit;

	if (mask == 0)
		return (0);
	for (bit = 1; mask != 1; bit++)
		mask = (unsigned long long)mask >> 1;
	return (bit);
}
#else
/*
 * fls64 - find leftmost set bit in a 64-bit word
 *
 * Returns 0 if no bit is set or the bit
 * position counted from 1 to 64 otherwise
 */
static inline int
fls64(__u64 x)
{
	return flsl(x);
}
#endif

#if defined(__OpenBSD__)
static inline int
__fls(long mask)
{
	return (flsl(mask) - 1);
}
#else
/* incorrect API? */
/* function ttm_pool_alloc() in ttm_pool.c */
static inline int
__fls(int mask)
{
	return (fls(mask) - 1);
}
#endif

static inline uint32_t
ror32(uint32_t word, unsigned int shift)
{
	return (word >> shift) | (word << (32 - shift));
}

#if 0
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
#endif

#define	ffz(mask)	__ffs(~(mask))

static inline int get_count_order(unsigned int count)
{
        int order;

        order = fls(count) - 1;
        if (count & (count - 1))
                order++;
        return order;
}

#if 0
#define	NBLONG	(NBBY * sizeof(long))
#endif

#if 0
#define	set_bit(i, a)							\
    atomic_set_long(&((volatile long *)(a))[(i)/NBLONG], 1LU << ((i) % NBLONG))

#define	clear_bit(i, a)							\
    atomic_clear_long(&((volatile long *)(a))[(i)/NBLONG], 1LU << ((i) % NBLONG))
#endif

#if 0
#define	test_bit(i, a)							\
    !!(atomic_load_acq_long(&((volatile long *)(a))[(i)/NBLONG]) &	\
			    (1LU << ((i) % NBLONG)))
#endif

#if 0
static inline long
test_and_clear_bit(long bit, long *var)
{
	long val;

	var += bit / (sizeof(long) * NBBY);
	bit %= sizeof(long) * NBBY;
	bit = 1L << bit;
	do {
		val = *(volatile long *)var;
	} while (atomic_cmpset_long(var, val, val & ~bit) == 0);

	return !!(val & bit);
}
#endif

#if 0
static inline unsigned long
__test_and_clear_bit(unsigned int bit, volatile unsigned long *ptr)
	unsigned int m = 1 << (b & 0x1f);
	unsigned int prev = __sync_fetch_and_and((volatile u_int *)p + (b >> 5), ~m);
	return (prev & m) != 0;
{
	const unsigned int units = (sizeof(*ptr) * NBBY);
	volatile unsigned long *const p = &ptr[bit / units];
	const unsigned long mask = (1UL << (bit % units));
	unsigned long v;

	v = *p;
	*p &= ~mask;

	return ((v & mask) != 0);
}
#endif

#if 0
static inline long
test_and_set_bit(long bit, volatile unsigned long *var)
{
	long val;

	var += bit / (sizeof(long) * NBBY);
	bit %= sizeof(long) * NBBY;
	bit = 1L << bit;
	do {
		val = *(volatile long *)var;
	} while (atomic_cmpset_long(var, val, val | bit) == 0);

	return !!(val & bit);
}
#endif

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) % BITS_PER_LONG))
#define BITMAP_LAST_WORD_MASK(nbits)                                    \
(                                                                       \
        ((nbits) % BITS_PER_LONG) ?                                     \
                (1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL               \
)

#if 0
static inline void
bitmap_set(unsigned long *map, int start, int nr)
{
	unsigned long *p = map + BIT_WORD(start);
	const int size = start + nr;
	int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);

	while (nr - bits_to_set >= 0) {
		*p |= mask_to_set;
		nr -= bits_to_set;
		bits_to_set = BITS_PER_LONG;
		mask_to_set = ~0UL;
		p++;
	}
	if (nr) {
		mask_to_set &= BITMAP_LAST_WORD_MASK(size);
		*p |= mask_to_set;
	}
}

static inline void
bitmap_clear(unsigned long *map, int start, int nr)
{
	unsigned long *p = map + BIT_WORD(start);
	const int size = start + nr;
	int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);

	while (nr - bits_to_clear >= 0) {
		*p &= ~mask_to_clear;
		nr -= bits_to_clear;
		bits_to_clear = BITS_PER_LONG;
		mask_to_clear = ~0UL;
		p++;
	}
	if (nr) {
		mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
		*p &= ~mask_to_clear;
	}
}
#endif

enum {
        REG_OP_ISFREE,          /* true if region is all zero bits */
        REG_OP_ALLOC,           /* set all bits in region */
        REG_OP_RELEASE,         /* clear all bits in region */
};

static int __reg_op(unsigned long *bitmap, int pos, int order, int reg_op)
{
        int nbits_reg;          /* number of bits in region */
        int index;              /* index first long of region in bitmap */
        int offset;             /* bit offset region in bitmap[index] */
        int nlongs_reg;         /* num longs spanned by region in bitmap */
        int nbitsinlong;        /* num bits of region in each spanned long */
        unsigned long mask;     /* bitmask for one long of region */
        int i;                  /* scans bitmap by longs */
        int ret = 0;            /* return value */

        /*
         * Either nlongs_reg == 1 (for small orders that fit in one long)
         * or (offset == 0 && mask == ~0UL) (for larger multiword orders.)
         */
        nbits_reg = 1 << order;
        index = pos / BITS_PER_LONG;
        offset = pos - (index * BITS_PER_LONG);
        nlongs_reg = BITS_TO_LONGS(nbits_reg);
        nbitsinlong = min(nbits_reg,  BITS_PER_LONG);

        /*
         * Can't do "mask = (1UL << nbitsinlong) - 1", as that
         * overflows if nbitsinlong == BITS_PER_LONG.
         */
        mask = (1UL << (nbitsinlong - 1));
        mask += mask - 1;
        mask <<= offset;

        switch (reg_op) {
        case REG_OP_ISFREE:
                for (i = 0; i < nlongs_reg; i++) {
                        if (bitmap[index + i] & mask)
                                goto done;
                }
                ret = 1;        /* all bits in region free (zero) */
                break;

        case REG_OP_ALLOC:
                for (i = 0; i < nlongs_reg; i++)
                        bitmap[index + i] |= mask;
                break;

        case REG_OP_RELEASE:
                for (i = 0; i < nlongs_reg; i++)
                        bitmap[index + i] &= ~mask;
                break;
        }
done:
        return ret;
}

/**
 * bitmap_find_free_region - find a contiguous aligned mem region
 *      @bitmap: array of unsigned longs corresponding to the bitmap
 *      @bits: number of bits in the bitmap
 *      @order: region size (log base 2 of number of bits) to find
 *
 * Find a region of free (zero) bits in a @bitmap of @bits bits and
 * allocate them (set them to one).  Only consider regions of length
 * a power (@order) of two, aligned to that power of two, which
 * makes the search algorithm much faster.
 *
 * Return the bit offset in bitmap of the allocated region,
 * or -errno on failure.
 */
static inline int 
bitmap_find_free_region(unsigned long *bitmap, int bits, int order)
{
        int pos, end;           /* scans bitmap by regions of size order */

        for (pos = 0 ; (end = pos + (1 << order)) <= bits; pos = end) {
                if (!__reg_op(bitmap, pos, order, REG_OP_ISFREE))
                        continue;
                __reg_op(bitmap, pos, order, REG_OP_ALLOC);
                return pos;
        }
        return -ENOMEM;
}

/**
 * bitmap_release_region - release allocated bitmap region
 *      @bitmap: array of unsigned longs corresponding to the bitmap
 *      @pos: beginning of bit region to release
 *      @order: region size (log base 2 of number of bits) to release
 *
 * This is the complement to __bitmap_find_free_region() and releases
 * the found region (by clearing it in the bitmap).
 *
 * No return value.
 */
static inline void 
bitmap_release_region(unsigned long *bitmap, int pos, int order)
{
        __reg_op(bitmap, pos, order, REG_OP_RELEASE);
}

// #include <asm/bitops/non-atomic.h>
// #include <asm/bitops/const_hweight.h>

#if 0
#define for_each_set_bit(bit, addr, size) \
	for ((bit) = find_first_bit((addr), (size));		\
	     (bit) < (size);					\
	     (bit) = find_next_bit((addr), (size), (bit) + 1))

#define for_each_clear_bit(bit, addr, size) \
	for ((bit) = find_first_zero_bit((addr), (size));	\
	     (bit) < (size);						\
	     (bit) = find_next_zero_bit((addr), (size), (bit) + 1))
#endif

#endif

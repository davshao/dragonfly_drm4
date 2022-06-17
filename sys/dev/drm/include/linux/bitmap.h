/*	$OpenBSD: bitmap.h,v 1.4 2022/06/15 07:04:09 jsg Exp $	*/
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

/*
 * Copyright (c) 2016-2019 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_BITMAP_H
#define _LINUX_BITMAP_H

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/string.h>
// #include <linux/kernel.h>

#if defined(__OpenBSD__)
#define bitmap_empty(p, n)	(find_first_bit(p, n) == n)
#else
static inline int
bitmap_empty(unsigned long *addr, int size)
{
	long mask;
	int tail;
	int len;
	int i;

	len = size / BITS_PER_LONG;
	for (i = 0; i < len; i++)
		if (addr[i] != 0)
			return (0);
	tail = size & (BITS_PER_LONG - 1);
	if (tail) {
		mask = BIT_MASK(tail);
		if ((addr[i] & mask) != 0)
			return (0);
	}
	return (1);
}
#endif

#if defined(__OpenBSD__)
static inline void
bitmap_set(void *p, int b, u_int n)
{
	u_int end = b + n;

	for (; b < end; b++)
		__set_bit(b, p);
}
#else
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
#endif

#if defined(__OpenBSD__)
static inline void
bitmap_clear(void *p, int b, u_int n)
{
	u_int end = b + n;

	for (; b < end; b++)
		__clear_bit(b, p);
}
#else
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

#if defined(__OpenBSD__)
static inline void
bitmap_zero(void *p, u_int n)
{
	u_int *ptr = p;
	u_int b;

	for (b = 0; b < n; b += 32)
		ptr[b >> 5] = 0;
}
#else
static inline void
bitmap_zero(unsigned long *addr, u_int n)
{
	int len;

	len = BITS_TO_LONGS(n) * sizeof(long);
	memset(addr, 0, len);
}
#endif

#if defined(__OpenBSD__)
static inline void
bitmap_or(void *d, void *s1, void *s2, u_int n)
{
	u_int *dst = d;
	u_int *src1 = s1;
	u_int *src2 = s2;
	u_int b;

	for (b = 0; b < n; b += 32)
		dst[b >> 5] = src1[b >> 5] | src2[b >> 5];
}
#else
static inline void
bitmap_or(unsigned long *dst, const unsigned long *src1,
	  const unsigned long *src2, unsigned int nbits)
{
	if (nbits <= BITS_PER_LONG) {
		*dst = *src1 | *src2;
	} else {
/* was DIV_ROUND_UP() in DragonFly which is the same as howmany() */
		int chunks = howmany(nbits, BITS_PER_LONG);

		for (int i = 0;i < chunks;i++)
			dst[i] = src1[i] | src2[i];
	}
}
#endif

static inline void
bitmap_andnot(void *d, void *s1, void *s2, u_int n)
{
	u_int *dst = d;
	u_int *src1 = s1;
	u_int *src2 = s2;
	u_int b;

	for (b = 0; b < n; b += 32)
		dst[b >> 5] = src1[b >> 5] & ~src2[b >> 5];
}

static inline void
bitmap_complement(void *d, void *s, u_int n)
{
        u_int *dst = d;
        u_int *src = s;
        u_int b;

        for (b = 0; b < n; b += 32)
                dst[b >> 5] = ~src[b >> 5];
}

static inline void
bitmap_copy(void *d, void *s, u_int n)
{
	u_int *dst = d;
	u_int *src = s;
	u_int b;

	for (b = 0; b < n; b += 32)
		dst[b >> 5] = src[b >> 5];
}

static inline void
bitmap_to_arr32(void *d, unsigned long *src, u_int n)
{
	u_int *dst = d;
	u_int b;

#ifdef __LP64__
	for (b = 0; b < n; b += 32) {
		dst[b >> 5] = src[b >> 6] & 0xffffffff;
		b += 32;
		if (b < n)
			dst[b >> 5] = src[b >> 6] >> 32;
	}
#else
	bitmap_copy(d, src, n);
#endif
	if ((n % 32) != 0)
		dst[n >> 5] &= (0xffffffff >> (32 - (n % 32)));
}

#if defined(__OpenBSD__)
static inline int
bitmap_weight(void *p, u_int n)
{
	u_int *ptr = p;
	u_int b;
	int sum = 0;

	for (b = 0; b < n; b += 32)
		sum += hweight32(ptr[b >> 5]);
	return sum;
}
#else
static inline int
bitmap_weight(unsigned long *bitmap, unsigned int nbits)
{
	unsigned int bit;
	unsigned int retval = 0;

	for_each_set_bit(bit, bitmap, nbits)
		retval++;
	return (retval);
}
#endif

void *bitmap_zalloc(u_int, gfp_t);
void bitmap_free(void *);

static inline void
bitmap_fill(unsigned long *addr, int size)
{
	int tail;
	int len;

	len = (size / BITS_PER_LONG) * sizeof(long);
	memset(addr, 0xff, len);
	tail = size & (BITS_PER_LONG - 1);
	if (tail) 
		addr[size / BITS_PER_LONG] = BIT_MASK(tail);
}

static inline int
bitmap_full(unsigned long *addr, int size)
{
	long mask;
	int tail;
	int len;
	int i;

	len = size / BITS_PER_LONG;
	for (i = 0; i < len; i++)
		if (addr[i] != ~0UL)
			return (0);
	tail = size & (BITS_PER_LONG - 1);
	if (tail) {
		mask = BIT_MASK(tail);
		if ((addr[i] & mask) != mask)
			return (0);
	}
	return (1);
}

#endif

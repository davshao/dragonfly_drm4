/* Public domain. */

/*
 * Copyright (c) 2015-2019 François Tigeot <ftigeot@wolfpond.org>
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
#ifndef	_LINUX_TYPES_H
#define	_LINUX_TYPES_H

#include <sys/types.h>
#include <sys/stdint.h>

// #include <uapi/linux/types.h>
// #include <linux/compiler.h>

typedef int8_t   __s8;
typedef uint8_t  __u8;
typedef int16_t  __s16;
typedef uint16_t __u16;
typedef int32_t  __s32;
typedef uint32_t __u32;
typedef int64_t  __s64;
typedef uint64_t __u64;

typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;

typedef uint16_t __le16; 
typedef uint16_t __be16; 
typedef uint32_t __le32; 
typedef uint32_t __be32;
typedef uint64_t __le64; 
typedef uint64_t __be64; 

#if defined(__OpenBSD__)
typedef bus_addr_t dma_addr_t;
typedef paddr_t phys_addr_t;
typedef paddr_t resource_size_t;
#else
typedef unsigned long dma_addr_t;
typedef unsigned long phys_addr_t;
typedef unsigned long resource_size_t;
#endif

#if defined(__OpenBSD__)
typedef off_t loff_t;
#else
typedef uint64_t loff_t;
#endif

typedef __ptrdiff_t ptrdiff_t;

typedef unsigned int gfp_t;

typedef unsigned long pgoff_t;

#if defined(__OpenBSD__)
typedef int pgprot_t;
#else
typedef unsigned long pgprot_t;
#endif

#if defined(__OpenBSD__)
typedef int atomic_t;
#else
typedef struct {
	volatile u_int counter;
} atomic_t;
#endif

struct list_head {
	struct list_head *next, *prev;
};

struct hlist_node {
#if defined(__OpenBSD__)
	struct hlist_node *next, **prev;
#else
	struct hlist_node *next, **pprev;
#endif
};

struct hlist_head {
	struct hlist_node *first;
};

#if defined(__OpenBSD__)
#define DECLARE_BITMAP(x, y)	unsigned long x[BITS_TO_LONGS(y)];
#else
#define DECLARE_BITMAP(n, bits) \
	unsigned long n[howmany(bits, sizeof(long) * 8)];
#endif

#define LINUX_PAGE_MASK		(~PAGE_MASK)

typedef unsigned short umode_t;

#if 0
struct rcu_head {
};
#endif

#if 0
typedef struct {
	long counter;
} atomic64_t;
#endif

#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat=1"
#pragma GCC diagnostic ignored "-Wformat=2"

#endif	/* _LINUX_TYPES_H_ */

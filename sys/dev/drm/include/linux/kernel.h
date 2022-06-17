/*
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2016 Mellanox Technologies, Ltd.
 * Copyright (c) 2014-2020 François Tigeot <ftigeot@wolfpond.org>
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
#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#include <sys/stdint.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/stdarg.h>

#include <sys/libkern.h>
#include <sys/stat.h>
#include <sys/malloc.h>

#include <machine/limits.h>	/* LONG_MAX etc... */

#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/bitops.h>
#include <linux/log2.h>

#include <linux/linkage.h>

#include <linux/printk.h>
#include <linux/typecheck.h>

#include <asm/byteorder.h>

#define swap(a, b) \
	do { __typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while(0)

#define container_of(ptr, type, member) ({			\
	__typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define offsetofend(s, e) (offsetof(s, e) + sizeof((((s *)0)->e)))

#define typeof_member(s, e)	typeof(((s *)0)->e)

#if 0
#define U8_MAX		((u8)~0U)
#define U16_MAX		((u16)~0U)
#define U32_MAX		((u32)~0U)
#define U64_MAX		((u64)~0ULL)

#define S8_MAX		((s8)(U8_MAX>>1))
#define S16_MAX		((s16)(U16_MAX>>1))
#define S32_MAX		((s32)(U32_MAX>>1))
#define S64_MAX		((s64)(U64_MAX>>1))
#endif

#define S8_MAX		INT8_MAX
#define S16_MAX		INT16_MAX
#define S32_MAX		INT32_MAX
#define S64_MAX		INT64_MAX

#define U8_MAX		UINT8_MAX
#define U16_MAX		UINT16_MAX
#define U32_MAX		UINT32_MAX

#ifndef U64_C
#if defined(__OpenBSD__)
#define U64_C(x)	UINT64_C(x)
#else
#define U64_C(x)	(x ## UL)	
#endif
#endif

#define U64_MAX		UINT64_MAX

#if defined(__OpenBSD__)
#define ARRAY_SIZE nitems
#else
#define	ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))
#endif

#define	lower_32_bits(n)	((u32)(n))
#define upper_32_bits(_val)	((u32)(((_val) >> 16) >> 16))

#if defined(__OpenBSD__)
#define scnprintf(str, size, fmt, arg...) snprintf(str, size, fmt, ## arg)

#else

#define snprintf	ksnprintf
#define sprintf		ksprintf

static inline int __printf(3, 0)
vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	return kvsnprintf(buf, size, fmt, args);
}

static inline int __printf(3, 0)
vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int ret;

	if (size == 0)
		return 0;

	ret = vsnprintf(buf, size, fmt, args);
	if (ret < size)
		return ret;

	return size - 1;
}

static inline int __printf(3, 4)
scnprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vscnprintf(buf, size, fmt, args);
	va_end(args);

	return (i);
}
#endif

#define min_t(t, a, b) ({ \
	t __min_a = (a); \
	t __min_b = (b); \
	__min_a < __min_b ? __min_a : __min_b; })

#define max_t(t, a, b) ({ \
	t __max_a = (a); \
	t __max_b = (b); \
	__max_a > __max_b ? __max_a : __max_b; })

#define clamp_t(t, x, a, b) min_t(t, max_t(t, x, a), b)
#define clamp(x, a, b) clamp_t(__typeof(x), x, a, b)
#define clamp_val(x, a, b) clamp_t(__typeof(x), x, a, b)

#if defined(__OpenBSD__)
#define min(a, b) MIN(a, b)
#define max(a, b) MAX(a, b)
#define min3(x, y, z) MIN(x, MIN(y, z))
#define max3(x, y, z) MAX(x, MAX(y, z))
#else
#define min(x, y)			((x) < (y) ? (x) : (y))
#define max(x, y)			((x) > (y) ? (x) : (y))
#define min3(a, b, c)			min(a, min(b,c))
#define max3(a, b, c)			max(a, max(b,c))
#endif

#define mult_frac(x, n, d) (((x) * (n)) / (d))

#ifndef roundup2
#define roundup2(x, y) (((x) + ((y) - 1)) & (~((__typeof(x))(y) - 1)))
#endif
#define round_up(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define round_down(x, y) (((x) / (y)) * (y)) /* y is power of two */
#ifndef rounddown
#define rounddown(x, y) (((x) / (y)) * (y)) /* arbitrary y */
#endif
#define DIV_ROUND_UP(x, y)	(((x) + ((y) - 1)) / (y))
#define DIV_ROUND_UP_ULL(x, y)	DIV_ROUND_UP(x, y)
#define DIV_ROUND_DOWN(x, y)	((x) / (y))
#define DIV_ROUND_DOWN_ULL(x, y)	DIV_ROUND_DOWN(x, y)
#define DIV_ROUND_CLOSEST(x, y)	(((x) + ((y) / 2)) / (y))
#define DIV_ROUND_CLOSEST_ULL(x, y)	DIV_ROUND_CLOSEST(x, y)

#define IS_ALIGNED(x, y)	(((x) & ((y) - 1)) == 0)
#define PTR_ALIGN(x, y)		((__typeof(x))roundup2((unsigned long)(x), (y)))

#undef	ALIGN
#define	ALIGN(x, y)		roundup2((x), (y))

#if defined(__OpenBSD__)
static inline char *
kvasprintf(int flags, const char *fmt, va_list ap)
{
	char *buf;
	size_t len;
	va_list vl;

	va_copy(vl, ap);
	len = vsnprintf(NULL, 0, fmt, vl);
	va_end(vl);

	buf = malloc(len + 1, M_DRM, flags);
	if (buf) {
		vsnprintf(buf, len + 1, fmt, ap);
	}

	return buf;
}

static inline char *
kasprintf(int flags, const char *fmt, ...)
{
	char *buf;
	va_list ap;

	va_start(ap, fmt);
	buf = kvasprintf(flags, fmt, ap);
	va_end(ap);

	return buf;
}

static inline int
vscnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
	int nc;

	nc = vsnprintf(buf, size, fmt, ap);
	if (nc > (size - 1))
		return (size - 1);
	else
		return nc;
}
#else
char *kvasprintf(int flags, const char *format, va_list ap);
char *kasprintf(int flags, const char *format, ...);
#endif

#if defined(__OpenBSD__)
static inline int
_in_dbg_master(void)
{
#ifdef DDB
	return (db_active);
#endif
	return (0);
}

#define oops_in_progress _in_dbg_master()
#else
#define oops_in_progress	(panicstr != NULL)
#endif

#if defined(__OpenBSD__)
#define might_sleep()		assertwaitok()
#define might_sleep_if(x)	do {	\
	if (x)				\
		assertwaitok();		\
} while (0)
#else
static inline void
might_sleep(void)
{
}

#define might_sleep_if(cond)
#endif

#define add_taint(x, y)
#define TAINT_MACHINE_CHECK	0
#define TAINT_WARN		1
#define LOCKDEP_STILL_OK	0

#define u64_to_user_ptr(x)	((void *)(uintptr_t)(x))

#define _RET_IP_		__builtin_return_address(0)

#if defined(__OpenBSD__)
#define STUB() do { printf("%s: stub\n", __func__); } while(0)
#else
#define STUB() do { kprintf("%s: stub\n", __func__); } while(0);
#endif

#if 0 /* moved to linux/delay.h */
#define udelay(t)       	DELAY(t)
#endif

#define	simple_strtoul	strtoul
#define	simple_strtol	strtol

#define	num_possible_cpus()	mp_ncpus

#if 0
typedef struct pm_message {
        int event;
} pm_message_t;
#endif

static inline int64_t abs64(int64_t x)
{
	return (x < 0 ? -x : x);
}

#if 0
#define	le16_to_cpu(x)	le16toh(x)
#define	le32_to_cpu(x)	le32toh(x)
#define	le64_to_cpu(x)	le64toh(x)

#define	be16_to_cpu(x)	be16toh(x)
#define	be32_to_cpu(x)	be32toh(x)
#define	be64_to_cpu(x)	be64toh(x)

#define	le16_to_cpup(x)	le16toh(*x)
#define	le32_to_cpup(x)	le32toh(*x)
#define	le64_to_cpup(x)	le64toh(*x)

#define	be16_to_cpup(x)	be16toh(*x)
#define	be32_to_cpup(x)	be32toh(*x)
#define	be64_to_cpup(x)	be64toh(*x)

#define	cpu_to_le16(x)	htole16(x)
#define	cpu_to_le32(x)	htole32(x)
#define	cpu_to_le64(x)	htole64(x)

#define	cpu_to_be16(x)	htobe16(x)
#define	cpu_to_be32(x)	htobe32(x)
#define	cpu_to_be64(x)	htobe64(x)
#endif

static inline int __must_check
kstrtouint(const char *s, unsigned int base, unsigned int *res)
{
	*(res) = strtol(s,0,base);

	return 0;
}

static inline int
kstrtol(const char *cp, unsigned int base, long *res)
{
	char *end;

	*res = strtol(cp, &end, base);

	/* skip newline character, if any */
	if (*end == '\n')
		end++;
	if (*cp == 0 || *end != 0)
		return (-EINVAL);
	return (0);
}

#endif

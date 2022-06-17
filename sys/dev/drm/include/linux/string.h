/* Public domain. */

/*
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013, 2014 Mellanox Technologies, Ltd.
 * Copyright (c) 2018 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_STRING_H
#define _LINUX_STRING_H

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/stdint.h>
#include <sys/errno.h>

#include <linux/types.h>

#if 0

#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <sys/stdarg.h>

#endif

#define __UNCONST(a)	((void *)(unsigned long)(const void *)(a))

#if defined(__OpenBSD__) || defined(__DragonFly__)
static inline void *
memchr_inv(const void *s, int c, size_t n)
{
	if (n != 0) {
		const unsigned char *p = s;

		do {
			if (*p++ != (unsigned char)c)
				return __UNCONST(p - 1);
		} while (--n != 0);
	}
	return (NULL);
}
#else /* previous DragonFly */
static inline void *
memchr_inv(const void *s, int c, size_t n)
{
	const uint8_t byte = c;	/* XXX lose */
	const char *p;

	for (p = s; n-- > 0; p++)
		if (*p != byte)
			return __UNCONST(p);

	return NULL;
}
#endif

static inline void *
memset32(uint32_t *b, uint32_t c, size_t len)
{
	uint32_t *dst = b;
	while (len--)
		*dst++ = c;
	return b;
}

static inline void *
memset64(uint64_t *b, uint64_t c, size_t len)
{
	uint64_t *dst = b;
#if defined(__OpenBSD__) || defined(__DragonFly__)
	while (len--)
		*dst++ = c;
#else /* previous DragonFly */
	__asm __volatile("rep stosq"
		: "+D" (dst), "+c" (len)	/* output operands (modified input) */
		: "a" (c)		/* unmodified input operands */
		: "memory");		/* clobbers */
#endif
	return b;
}

static inline void *
memset_p(void **p, void *v, size_t n)
{
#if defined(__LP64__) || defined(__DragonFly__) 
	return memset64((uint64_t *)p, (uintptr_t)v, n);
#else
	return memset32((uint32_t *)p, (uintptr_t)v, n);
#endif
}

void *kmemdup(const void *src, size_t len, gfp_t gfp);

#if defined(__OpenBSD__)
static inline void *
kstrdup(const char *str, int flags)
{
	size_t len;
	char *p;

	if (str == NULL)
		return NULL;

	len = strlen(str) + 1;
	p = kmalloc(len, M_DRM, flags);
	if (p)
		memcpy(p, str, len);
	return (p);
}
#else
/* DragonFly declared as kstrdup(str, type) in sys/malloc.h */
#endif

static inline int
match_string(const char * const *array,  size_t n, const char *str)
{
	int i;

	for (i = 0; i < n; i++) {
		if (array[i] == NULL)
			break;
		if (!strcmp(array[i], str))	
			return i;
	}

	return -EINVAL;
}

#if 0
#include <asm/string_64.h>	/* for memset64() */
#endif

#endif

/*	$OpenBSD: bug.h,v 1.2 2021/07/07 02:38:36 jsg Exp $	*/
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

#ifndef _LINUX_BUG_H
#define _LINUX_BUG_H

// #include <asm/bug.h>

// #include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <linux/compiler.h>
#include <linux/build_bug.h>

// #include <linux/kernel.h>

#define BUG()								\
do {									\
	panic("BUG in %s at %s:%u", __func__, __FILE__, __LINE__);	\
} while (0)

#if defined(__OpenBSD__)
#ifndef DIAGNOSTIC
#define BUG_ON(x)	((void)(x))
#else
#define BUG_ON(x)	KASSERT(!(x))
#endif
#else
#define BUG_ON(condition)	do { if (condition) BUG(); } while(0)
#endif

#ifndef WARN
#define WARN(condition, fmt...) ({					\
	int __ret = !!(condition);				\
	if (__ret)					\
		pr_warning(fmt);					\
	unlikely(__ret);					\
})
#endif

#define WARN_ONCE(condition, fmt...) ({					\
	static int __warned;					\
	int __ret = !!(condition);					\
									\
	if (__ret && !__warned) {				\
		WARN(condition, fmt);					\
		__warned = 1;					\
	}								\
	unlikely(__ret);						\
})

#define _WARN_STR(x) #x

#define WARN_ON(condition) ({						\
	int __ret = !!(condition);					\
	if (__ret)							\
		kprintf("WARNING %s failed at %s:%d\n",			\
		    _WARN_STR(condition), __FILE__, __LINE__);		\
	unlikely(__ret);						\
})

#define WARN_ON_ONCE(condition) ({					\
	static int __warned;						\
	int __ret = !!(condition);					\
	if (__ret && !__warned) {					\
		kprintf("WARNING %s failed at %s:%d\n",			\
		    _WARN_STR(condition), __FILE__, __LINE__);		\
		__warned = 1;						\
	}								\
	unlikely(__ret);						\
})

#define WARN_ON_SMP(condition)	WARN_ON(condition)

#if 0
/*
 * Some conditions result in a "expression in static assertion is not constant"
 * compilation error; just disable the check for now.
 */
#define BUILD_BUG_ON(condition)

#define	BUILD_BUG_ON_NOT_POWER_OF_2(n)			      \
	CTASSERT(((n) != 0) && (powerof2((n))))

#define	BUILD_BUG()	BUILD_BUG_ON(1)

#define	BUILD_BUG_ON_MSG(cond, msg)	do { } while (0)

#define BUILD_BUG_ON_INVALID(expr)	((void)(expr))

#define BUILD_BUG_ON_ZERO(x)	0
#endif

#endif

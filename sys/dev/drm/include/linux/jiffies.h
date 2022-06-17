/* Public domain. */

/*
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

#ifndef _LINUX_JIFFIES_H
#define _LINUX_JIFFIES_H

#include <sys/kernel.h>
#include <machine/limits.h>

#include <linux/math64.h>
#include <linux/time.h>

#if defined(__OpenBSD__)
extern volatile unsigned long jiffies;
#else
#define jiffies			ticks
#endif

#if defined(__OpenBSD__)
#define jiffies_64 jiffies /* XXX */
#else
#define jiffies_64 ticks /* XXX hmmm */
#endif

#undef HZ
#define HZ			hz

#if defined(__OpenBSD__)
#define MAX_JIFFY_OFFSET	((INT_MAX >> 1) - 1)
#else
#define MAX_JIFFY_OFFSET	((LONG_MAX >> 1) - 1)
#endif

#if defined(__OpenBSD__) || defined(__DragonFly__)
#define time_in_range(x, min, max) ((x) >= (min) && (x) <= (max))
#else
#define time_in_range(x, min, max)	\
	time_after_eq(x, min) && time_before_eq(x, max)
#endif

#if defined(__OpenBSD__)
static inline unsigned int
jiffies_to_msecs(const unsigned long x)
{
	return (((uint64_t)(x)) * 1000 / hz);
}
#else
#define jiffies_to_msecs(x)	(((int64_t)(x)) * 1000 / hz)
#endif

#if defined(__OpenBSD__)
static inline unsigned int
jiffies_to_usecs(const unsigned long x)
{
	return (((uint64_t)(x)) * 1000000 / hz);
}
#else
static inline unsigned int
jiffies_to_usecs(const unsigned long x)
{
	return x * (USEC_PER_SEC / HZ);
}
#endif

#if defined(__OpenBSD__)
#define msecs_to_jiffies(x)	(((uint64_t)(x)) * hz / 1000)
#else
#define msecs_to_jiffies(x)	(((int64_t)(x)) * hz / 1000)
#endif

#if defined(__OpenBSD__)
#define usecs_to_jiffies(x)	(((uint64_t)(x)) * hz / 1000000)
#else
static inline
unsigned long usecs_to_jiffies(const unsigned int x)
{
	unsigned long jiffies;

	jiffies = (x * hz) / 1000000;
	if (jiffies < 1)
		return 1;
	else
		return jiffies;
}
#endif

#if defined(__OpenBSD__)
#define nsecs_to_jiffies(x)	(((uint64_t)(x)) * hz / 1000000000)
#else
static inline u64
nsecs_to_jiffies(u64 x)
{
	return (x * hz) / NSEC_PER_SEC;
}
#endif

#if defined(__OpenBSD__)
#define nsecs_to_jiffies64(x)	(((uint64_t)(x)) * hz / 1000000000)
#else
static inline u64
nsecs_to_jiffies64(u64 x)
{
	return nsecs_to_jiffies(x);
}
#endif

#if defined(__OpenBSD__)
#define get_jiffies_64()	jiffies
#else
static inline u64
get_jiffies_64(void)
{
	return (u64)ticks;
}
#endif

#define time_after(a,b)		((long)(b) - (long)(a) < 0)
#define time_after32(a,b)	((int32_t)((uint32_t)(b) - (uint32_t)(a)) < 0)
#define time_after_eq(a,b)	((long)(b) - (long)(a) <= 0)

#if defined(__OpenBSD__)
#define time_before(a,b)	((long)(a) - (long)(b) < 0)
#else
#define time_before(a,b)	time_after(b,a)
#endif

#define time_before_eq(a,b)	time_after_eq(b,a)

static inline unsigned long
timespec_to_jiffies(const struct timespec *ts)
{
	unsigned long result;

	result = ((unsigned long)hz * ts->tv_sec) + (ts->tv_nsec / NSEC_PER_SEC);
	if (result > LONG_MAX)
		result = LONG_MAX;

	return result;
}

static inline u64
jiffies_to_nsecs(const unsigned long j)
{
	return j * (NSEC_PER_SEC / HZ);
}

#endif

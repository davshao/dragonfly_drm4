/* Public domain. */

/*
 * Copyright (c) 2014 François Tigeot
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

#ifndef _LINUX_DELAY_H
#define _LINUX_DELAY_H

#include <sys/param.h>
#if defined(__OpenBSD__)
#include <linux/kernel.h>
#else
#include <sys/systm.h> /* declares void DELAY(int usec) */
#include <linux/jiffies.h>
#endif

static inline void
udelay(unsigned long usecs)
{
	DELAY(usecs);
}

static inline void
ndelay(unsigned long nsecs)
{
#if defined(__OpenBSD__)
	DELAY(MAX(nsecs / 1000, 1));
#else
	DELAY(howmany(nsecs, 1000));
#endif
}

static inline void
usleep_range(unsigned long min, unsigned long max)
{
#if defined(__OpenBSD__)
	DELAY((min + max) / 2);
#else /* not sure API question */
	DELAY(min);
#endif
}

static inline void
mdelay(unsigned long msecs)
{
	int loops = msecs;
	while (loops--)
		DELAY(1000);
}

#define drm_msleep(x)		mdelay(x)

static inline unsigned int
msleep_interruptible(unsigned int msecs)
{
#if defined(__OpenBSD__)
	int r = tsleep_nsec(&nowake, PWAIT|PCATCH, "msleepi",
	    MSEC_TO_NSEC(msecs));
#else
	static int dummy;
	int delay = MAX(msecs*hz/1000, 1);
	int r = tsleep(&dummy, PCATCH, "lmslint", delay);
#endif
	if (r == EINTR)
		return 1;
	return 0;
}

static inline void
msleep(unsigned int msecs)
{
	static int dummy;
	int delay = MAX(msecs*hz/1000, 1);

	tsleep(&dummy, 0, "linux_msleep", delay);
}

#endif

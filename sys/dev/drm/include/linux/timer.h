/*	$OpenBSD: timer.h,v 1.8 2021/07/14 09:56:17 jsg Exp $	*/
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
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
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
#ifndef _LINUX_TIMER_H
#define _LINUX_TIMER_H

#include <sys/types.h>
#include <sys/param.h>
#include <sys/callout.h>
#include <sys/kernel.h>

// #include <linux/list.h>
#include <linux/ktime.h>
// #include <linux/stddef.h>
// #include <linux/stringify.h>

struct timer_list {
	struct callout	timer_callout;
	void		(*function)(unsigned long);
	unsigned long	data;
	unsigned long	expires;
};

static inline void
_timer_fn(void *context)
{
	struct timer_list *timer;

	timer = context;
	timer->function(timer->data);
}

#if defined(__OpenBSD__)

#define del_timer_sync(x)	timeout_del_barrier((x))
#define del_timer(x)		timeout_del((x))
#define timer_pending(x)	timeout_pending((x))

#else

static inline int
del_timer_sync(struct timer_list *timer)
{
	return callout_drain(&(timer)->timer_callout) == 0;
}

static inline void
del_timer(struct timer_list *timer)
{
	callout_stop(&(timer)->timer_callout);
}

#define	timer_pending(timer)	callout_pending(&(timer)->timer_callout)

#endif

#if defined(__OpenBSD__)

static inline int
mod_timer(struct timeout *to, unsigned long j)
{
	if (j <= jiffies)
		return timeout_add(to, 1);
	return timeout_add(to, j - jiffies);
}

static inline unsigned long
round_jiffies_up(unsigned long j)
{
	return roundup(j, hz);
}

static inline unsigned long
round_jiffies_up_relative(unsigned long j)
{
	return roundup(j, hz);
}

#else

#define	mod_timer(timer, exp)						\
do {									\
	(timer)->expires = (exp);					\
	callout_reset(&(timer)->timer_callout, (exp) - jiffies,		\
	    _timer_fn, (timer));					\
} while (0)

static inline unsigned long
round_jiffies_up(unsigned long j)
{
	return roundup(j, hz);
}

static inline unsigned long
round_jiffies_up_relative(unsigned long j)
{
	return roundup(j, hz);
}

#endif

#define	setup_timer(timer, func, dat)					\
do {									\
	(timer)->function = (func);					\
	(timer)->data = (dat);						\
	callout_init_mp(&(timer)->timer_callout);			\
} while (0)

#define TIMER_IRQSAFE	0x00200000

#define from_timer(var, arg, field)			\
        container_of(arg, typeof(*(var)), field)

/* OpenBSD uses its own timeout_set() */

static inline void
timer_setup(struct timer_list *timer,
	    void (*callback)(struct timer_list *),
	    unsigned int flags)
{
	setup_timer(timer,
	    (void (*)(unsigned long))callback,
	    (unsigned long)timer);
}

#if 0
#define setup_timer_on_stack(t, f,d)	setup_timer(t, f, d)

#define __setup_timer(timer, func, data, flags)				\
do {									\
	setup_timer(timer, func, data);					\
} while (0)

#define	init_timer(timer)						\
do {									\
	(timer)->function = NULL;					\
	(timer)->data = 0;						\
	callout_init_mp(&(timer)->timer_callout);			\
} while (0)

#define mod_timer_pinned(timer, exp)	mod_timer(timer, exp)

#define	add_timer(timer)						\
	callout_reset(&(timer)->timer_callout,				\
	    (timer)->expires - jiffies, _timer_fn, (timer));		\

#define del_singleshot_timer_sync(timer)	del_timer_sync(timer)

static inline unsigned long
round_jiffies(unsigned long j)
{
	return roundup(j, hz);
}

#define destroy_timer_on_stack(timer)

#endif

#endif

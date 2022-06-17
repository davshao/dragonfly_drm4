/* Public domain. */

/*
 * Copyright (c) 2017-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_INTERRUPT_H
#define _LINUX_INTERRUPT_H

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/preempt.h>
#include <linux/cpumask.h>
#include <linux/hardirq.h>
#include <linux/irqflags.h>
#include <linux/hrtimer.h>
#include <linux/kref.h>

#include <linux/atomic.h>
#include <linux/compiler.h>
#include <linux/irqreturn.h>

#define IRQF_SHARED	0x00000080

#if defined(__OpenBSD__)

#define request_irq(irq, hdlr, flags, name, dev)	(0)

static inline void
free_irq(unsigned int irq, void *dev)
{
}

typedef irqreturn_t (*irq_handler_t)(int, void *);

#else

typedef irqreturn_t (*irq_handler_t)(int, void *);

int request_irq(unsigned int irq, irq_handler_t handler,
		unsigned long flags, const char *name, void *dev);

void free_irq(unsigned int irq, void *dev_id);

void disable_irq(unsigned int irq);
void enable_irq(unsigned int irq);

#endif

struct tasklet_struct {
	void (*func)(unsigned long);
	void (*callback)(struct tasklet_struct *);
	unsigned long data;
	unsigned long state;
	atomic_t count;
};

#if defined(__OpenBSD_)
#define TASKLET_STATE_SCHED	1
#define TASKLET_STATE_RUN	0
#else
enum {
	TASKLET_STATE_SCHED,
	TASKLET_STATE_RUN,
	TASKLET_IS_DYING
};
#endif

#define from_tasklet(x, t, f) \
	container_of(t, typeof(*x), f)

void
tasklet_init(struct tasklet_struct *ts, void (*func)(unsigned long),
    unsigned long data);

void
tasklet_setup(struct tasklet_struct *ts,
    void (*callback)(struct tasklet_struct *));

void tasklet_schedule(struct tasklet_struct *t);
void tasklet_hi_schedule(struct tasklet_struct *t);
void tasklet_kill(struct tasklet_struct *t);

static inline void
tasklet_disable(struct tasklet_struct *ts)
{
	atomic_inc(&ts->count);
}

static inline void 
tasklet_disable_nosync(struct tasklet_struct *ts)
{
	atomic_inc(&ts->count);
#if defined(__OpenBSD__) /* does nothing on __amd64__ */
	smp_mb__after_atomic();
#endif
}

static inline void
tasklet_enable(struct tasklet_struct *ts)
{
#if defined(__OpenBSD__) /* does nothing on __amd64__ */
	smp_mb__before_atomic();
#endif
	atomic_dec(&ts->count);
}

#endif

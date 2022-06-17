/*	$OpenBSD: irq_work.h,v 1.9 2022/07/27 07:08:34 jsg Exp $	*/
/*
 * Copyright (c) 2015 Mark Kettenis
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
 * Copyright (c) 2020 François Tigeot <ftigeot@wolfpond.org>
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
 * DATA, OR PROFITS; OR BUSINESS INTIRQ_WORKUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	_LINUX_IRQ_WORK_H
#define	_LINUX_IRQ_WORK_H

#if defined(__OpenBSD__)
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/task.h>
#else
#include <linux/workqueue.h>
#endif

#include <linux/llist.h>

struct irq_node {
	struct llist_node llist;
};

struct irq_work {
#if defined(__OpenBSD__)
	struct task task;
	struct taskq *tq;
	struct irq_node node;
#else
	struct work_struct ws;
#endif
};

typedef void (*irq_work_func_t)(struct irq_work *);

static inline void
init_irq_work(struct irq_work *work, irq_work_func_t func)
// init_irq_work(struct irq_work *work, void (*func)(struct irq_work *))
{
#if defined(__OpenBSD__)
	work->tq = (struct taskq *)system_wq;
	task_set(&work->task, (void (*)(void *))func, work);
#else
	INIT_WORK(&work->ws, (void (*)(struct work_struct *))func);
#endif
}

static inline bool
irq_work_queue(struct irq_work *work)
{
#if defined(__OpenBSD__)
	return task_add(work->tq, &work->task);
#else
	/* XXX: irq_works are supposed to be processed in hardirq context */
	queue_work(system_highpri_wq, &work->ws);

	return true;
#endif
}

static inline void
irq_work_sync(struct irq_work *work)
{
#if defined(__OpenBSD__)
	taskq_barrier(work->tq);
#else
/* not implemented on DragonFly */
#endif
}

#endif

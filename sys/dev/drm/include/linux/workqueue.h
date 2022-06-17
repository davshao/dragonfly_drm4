/*	$OpenBSD: workqueue.h,v 1.8 2022/03/01 04:08:04 jsg Exp $	*/
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
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013, 2014 Mellanox Technologies, Ltd.
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
#ifndef	_LINUX_WORKQUEUE_H
#define	_LINUX_WORKQUEUE_H

#include <sys/param.h>
#include <sys/systm.h>
#if defined(__OpenBSD__)
#include <sys/task.h>
#include <sys/timeout.h>
#else
#include <sys/thread.h>
#include <sys/callout.h>
#endif
#include <linux/bitops.h>
#include <linux/atomic.h>
#include <linux/rcupdate.h>
#include <linux/kernel.h>
#include <linux/lockdep.h>
#include <linux/timer.h>
#include <linux/cpumask.h>

struct workqueue_struct;

extern struct workqueue_struct *system_wq;
extern struct workqueue_struct *system_highpri_wq;
extern struct workqueue_struct *system_unbound_wq;
extern struct workqueue_struct *system_long_wq;

#define WQ_HIGHPRI	1
#define WQ_FREEZABLE	2
#define WQ_UNBOUND	4

#define WQ_UNBOUND_MAX_ACTIVE	4	/* matches nthreads in drm_linux.c */

#if defined(__OpenBSD__)
static inline struct workqueue_struct *
alloc_workqueue(const char *name, int flags, int max_active)
{
	struct taskq *tq = taskq_create(name, 1, IPL_TTY, 0);
	return (struct workqueue_struct *)tq;
}
#else
#define alloc_workqueue(name, flags, max_active) \
	_create_workqueue_common(name, flags)
#endif

#if defined(__OpenBSD__)
static inline struct workqueue_struct *
alloc_ordered_workqueue(const char *name, int flags)
{
	struct taskq *tq = taskq_create(name, 1, IPL_TTY, 0);
	return (struct workqueue_struct *)tq;
}
#else
#define alloc_ordered_workqueue(name, flags) \
	_create_workqueue_common(name, (flags) | WQ_UNBOUND)
#endif

#if defined(__OpenBSD__)
static inline struct workqueue_struct *
create_singlethread_workqueue(const char *name)
{
	struct taskq *tq = taskq_create(name, 1, IPL_TTY, 0);
	return (struct workqueue_struct *)tq;
}
#else
#define create_singlethread_workqueue(name) \
	_create_workqueue_common(name, WQ_UNBOUND)
#endif

struct workqueue_struct *_create_workqueue_common(const char *name, int flags);

#if defined(__OpenBSD__)
static inline void
destroy_workqueue(struct workqueue_struct *wq)
{
	taskq_destroy((struct taskq *)wq);
}
#else
void destroy_workqueue(struct workqueue_struct *wq);
#endif

struct workqueue_worker;

struct work_struct {
#if defined(__OpenBSD__)
	struct task task;
	struct taskq *tq;
#else
	STAILQ_ENTRY(work_struct) ws_entries;
	void	(*func)(struct work_struct *);
	struct workqueue_worker *worker;
	bool	on_queue;
	bool	running;
	bool	canceled;
#endif
};

typedef void (*work_func_t)(struct work_struct *);

struct workqueue_worker {
	STAILQ_HEAD(ws_list, work_struct) ws_list_head;
	struct thread *worker_thread;
	struct lock worker_lock;
};

struct workqueue_struct {
	bool	is_draining;
	int	num_workers;
	struct	workqueue_worker (*workers)[];
};

#if defined(__OpenBSD__)
static inline void
INIT_WORK(struct work_struct *work, work_func_t func)
{
	work->tq = NULL;
	task_set(&work->task, (void (*)(void *))func, work);
}
#else
#define INIT_WORK(work, _func) 		 	\
do {						\
	(work)->ws_entries.stqe_next = NULL;	\
	(work)->func = (_func);			\
	(work)->on_queue = false;		\
	(work)->running = false;		\
	(work)->canceled = false;		\
} while (0)
#endif

#define INIT_WORK_ONSTACK(x, y)	INIT_WORK((x), (y))

static inline bool
queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
#if defined(__OpenBSD__)
	work->tq = (struct taskq *)wq;
	return task_add(work->tq, &work->task);
#else
/*
 * Return false if work was already on a queue
 * Return true and queue it if this was not the case
 */
	struct workqueue_worker *worker;
	int ret = false;

	/* XXX: should we block instead ? */
	if (wq->is_draining)
		return false;

	if (wq->num_workers > 1)
		worker = &(*wq->workers)[mycpuid];
	else
		worker = &(*wq->workers)[0];

	lockmgr(&worker->worker_lock, LK_EXCLUSIVE);
	work->canceled = false;
	if (work->on_queue == false || work->running == false) {
		if (work->on_queue == false) {
			STAILQ_INSERT_TAIL(&worker->ws_list_head, work,
					   ws_entries);
			work->on_queue = true;
			work->worker = worker;
			wakeup_one(worker);
		}
		ret = true;
	}
	lockmgr(&worker->worker_lock, LK_RELEASE);

	return ret;
#endif
}

static inline bool
_cancel_work(struct work_struct *work, bool sync_wait)
{
	struct workqueue_worker *worker;
	bool ret;

	ret = false;

	for (;;) {
		if (work->on_queue) {
			worker = work->worker;
			if (worker == NULL)
				continue;
			lockmgr(&worker->worker_lock, LK_EXCLUSIVE);
			if (worker != work->worker || work->on_queue == false) {
				lockmgr(&worker->worker_lock, LK_RELEASE);
				continue;
			}
			STAILQ_REMOVE(&worker->ws_list_head, work,
				      work_struct, ws_entries);
			work->on_queue = false;
			ret = true;
			lockmgr(&worker->worker_lock, LK_RELEASE);
		}
		if (work->running == false)
			break;

		worker = work->worker;
		if (worker == NULL)
			continue;
		lockmgr(&worker->worker_lock, LK_EXCLUSIVE);
		if (worker != work->worker || work->running == false) {
			lockmgr(&worker->worker_lock, LK_RELEASE);
			continue;
		}
		work->canceled = true;
		ret = true;
		if (sync_wait == false) {
			lockmgr(&worker->worker_lock, LK_RELEASE);
			break;
		}
		/* XXX this races */
		lksleep(work, &worker->worker_lock, 0, "wqcan", 1);
		lockmgr(&worker->worker_lock, LK_RELEASE);
		/* retest */
	}

	return ret;
}

static inline void
cancel_work_sync(struct work_struct *work)
{
#if defined(__OpenBSD__)
	if (work->tq != NULL)
		task_del(work->tq, &work->task);
#else
/*
 * If work was queued, remove it from the queue and return true.
 * If work was not queued, return false.
 * In any case, wait for work to complete or be removed from the workqueue,
 * callers may free associated data structures after this call.
 */
	_cancel_work(work, true);
#endif
}

#if defined(__OpenBSD__)
#define work_pending(work)	task_pending(&(work)->task)
#else
bool work_pending(struct work_struct *work);
#endif

struct delayed_work {
	struct work_struct work;
#if defined(__OpenBSD__)
	struct timeout to;
	struct taskq *tq;
#else
	struct callout timer;
#endif
};

#if defined(__OpenBSD__)
#define system_power_efficient_wq ((struct workqueue_struct *)systq)
#else
extern struct workqueue_struct *system_power_efficient_wq;
#endif

static inline struct delayed_work *
to_delayed_work(struct work_struct *work)
{
	return container_of(work, struct delayed_work, work);
}

static inline void
INIT_DELAYED_WORK(struct delayed_work *dwork, work_func_t func)
{
	INIT_WORK(&dwork->work, func);
#if defined(__OpenBSD__)
	timeout_set(&dwork->to, __delayed_work_tick, &dwork->work);
#else
	callout_init_mp(&(dwork)->timer);
#endif
}

static inline void
INIT_DELAYED_WORK_ONSTACK(struct delayed_work *dwork, work_func_t func)
{
#if defined(__OpenBSD__)
	INIT_WORK(&dwork->work, func);
	timeout_set(&dwork->to, __delayed_work_tick, &dwork->work);
#else
	INIT_DELAYED_WORK(dwork, func);
#endif
}

#if 0
#define INIT_DELAYED_WORK(dwork, func)					\
do {									\
	INIT_WORK(&(dwork)->work, func);				\
	callout_init_mp(&(dwork)->timer);				\
} while (0)

#define INIT_DELAYED_WORK_ONSTACK(work, _func)	INIT_DELAYED_WORK(work, _func)
#endif

static inline bool
schedule_work(struct work_struct *work)
{
#if defined(__OpenBSD__)
	work->tq = (struct taskq *)system_wq;
	return task_add(work->tq, &work->task);
#else
	return queue_work(system_wq, work);
#endif
}

// int queue_work(struct workqueue_struct *wq, struct work_struct *work);

#if defined(__OpenBSD__)
static inline bool
queue_delayed_work(struct workqueue_struct *wq,
    struct delayed_work *dwork, int jiffies);
#else
static inline bool
queue_delayed_work(struct workqueue_struct *wq,
    struct delayed_work *dwork, unsigned long jiffies);
#endif

static inline bool
#if defined(__OpenBSD__)
schedule_delayed_work(struct delayed_work *dwork, int jiffies)
#else
schedule_delayed_work(struct delayed_work *dwork, unsigned long jiffies)
#endif
{
#if defined(__OpenBSD__)
	dwork->tq = (struct taskq *)system_wq;
	return timeout_add(&dwork->to, jiffies);
#else
        return queue_delayed_work(system_wq, dwork, jiffies);
#endif
}

static inline void
_delayed_work_fn(void *arg)
{
	struct delayed_work *dw = arg;

	queue_work(system_wq, &dw->work);
}

static inline bool
#if defined(__OpenBSD__)
queue_delayed_work(struct workqueue_struct *wq,
    struct delayed_work *dwork, int jiffies)
#else
queue_delayed_work(struct workqueue_struct *wq,
    struct delayed_work *dwork, unsigned long jiffies)
#endif
{
#if defined(__OpenBSD__)
	dwork->tq = (struct taskq *)wq;
	return timeout_add(&dwork->to, jiffies);
#else
	int pending = dwork->work.on_queue; // XXX: running too ?
	if (jiffies != 0) {
		callout_reset(&dwork->timer, jiffies, _delayed_work_fn, dwork);
	} else {
		_delayed_work_fn((void *)dwork);
	}

	return (!pending);
#endif
}

// bool cancel_work_sync(struct work_struct *work);
bool cancel_delayed_work(struct delayed_work *dwork);
bool cancel_delayed_work_sync(struct delayed_work *dwork);

/* XXX: Return value not used in drm code */
static inline bool
#if defined(__OpenBSD__)
mod_delayed_work(struct workqueue_struct *wq,
		 struct delayed_work *dwork, int jiffies)
#else
mod_delayed_work(struct workqueue_struct *wq,
		 struct delayed_work *dwork, unsigned long jiffies)
#endif
{
#if defined(__OpenBSD__)
	dwork->tq = (struct taskq *)wq;
	return (timeout_add(&dwork->to, jiffies) == 0);
#else
	cancel_delayed_work(dwork);
	queue_delayed_work(wq, dwork, jiffies);
	return false;
#endif
}

#if defined(__OpenBSD__)
static inline bool
cancel_delayed_work(struct delayed_work *dwork)
{
	if (dwork->tq == NULL)
		return false;
	if (timeout_del(&dwork->to))
		return true;
	return task_del(dwork->tq, &dwork->work.task);
}

static inline bool
cancel_delayed_work_sync(struct delayed_work *dwork)
{
	if (dwork->tq == NULL)
		return false;
	if (timeout_del(&dwork->to))
		return true;
	return task_del(dwork->tq, &dwork->work.task);
}
#endif

#if defined(__OpenBSD__)
static inline bool
delayed_work_pending(struct delayed_work *dwork)
{
	if (timeout_pending(&dwork->to))
		return true;
	return task_pending(&dwork->work.task);
}
#else
bool
delayed_work_pending(struct delayed_work *dwork);
#endif

void flush_workqueue(struct workqueue_struct *);
bool flush_work(struct work_struct *);
bool flush_delayed_work(struct delayed_work *);

static inline void
flush_scheduled_work(void)
{
	flush_workqueue(system_wq);
}

#if defined(__OpenBSD__)
static inline void
drain_workqueue(struct workqueue_struct *wq)
{
	flush_workqueue(wq);
}
#else
void
drain_workqueue(struct workqueue_struct *wq);
#endif

#if defined(__OpenBSD__)
static inline void
destroy_work_on_stack(struct work_struct *work)
{
	if (work->tq)
		task_del(work->tq, &work->task);
}
#else
void
destroy_work_on_stack(struct work_struct *work);
#endif

#if defined(__OpenBSD__)
static inline void
destroy_delayed_work_on_stack(struct delayed_work *dwork)
{
}
#else
void
destroy_delayed_work_on_stack(struct delayed_work *work);
#endif

struct rcu_work {
	struct work_struct work;
	struct rcu_head rcu;
};

static inline void
INIT_RCU_WORK(struct rcu_work *work, work_func_t func)
{
	INIT_WORK(&work->work, func);
}

static inline bool
queue_rcu_work(struct workqueue_struct *wq, struct rcu_work *work)
{
	return queue_work(wq, &work->work);
}

unsigned int work_busy(struct work_struct *work);

#endif

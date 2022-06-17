/*	$OpenBSD: wait.h,v 1.8 2021/07/07 02:38:36 jsg Exp $	*/
/*
 * Copyright (c) 2013, 2014, 2015 Mark Kettenis
 * Copyright (c) 2017 Martin Pieuchot
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
 * Copyright (c) 2014 Imre Vadász
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

#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H

#include <sys/systm.h>
#include <sys/lock.h>

#include <linux/list.h>
// #include <linux/stddef.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <asm/current.h>

struct wait_queue_entry {
	unsigned int flags;
	void *private;
	int (*func)(struct wait_queue_entry *, unsigned, int, void *);
	struct list_head entry;
};

typedef struct wait_queue_entry wait_queue_entry_t;

#if defined(__OpenBSD__)
extern struct mutex sch_mtx;
extern volatile struct proc *sch_proc;
extern volatile void *sch_ident;
extern int sch_priority;
#endif

struct wait_queue_head {
	struct lock lock;
	struct list_head head;
};
typedef struct wait_queue_head wait_queue_head_t;

static inline void
init_waitqueue_head(wait_queue_head_t *wqh)
{
	lockinit(&wqh->lock, "lwq", 0, 0);
	INIT_LIST_HEAD(&wqh->head);
}
 
#define __init_waitqueue_head(wqh, name, key)	init_waitqueue_head(wqh)

int default_wake_function(wait_queue_entry_t *wait, unsigned mode, int flags, void *key);
int autoremove_wake_function(struct wait_queue_entry *, unsigned int, int, void *);

static inline void
init_wait_entry(wait_queue_entry_t *wqe, int flags)
{
	wqe->flags = flags;
	wqe->private = current;
	wqe->func = autoremove_wake_function;
	INIT_LIST_HEAD(&wqe->entry);
}

static inline void
__add_wait_queue(wait_queue_head_t *wqh, wait_queue_entry_t *wqe)
{
	list_add(&wqe->entry, &wqh->head);
}

static inline void
__add_wait_queue_entry_tail(wait_queue_head_t *wqh, wait_queue_entry_t *wqe)
{
	list_add_tail(&wqe->entry, &wqh->head);
}

static inline void
add_wait_queue(wait_queue_head_t *head, wait_queue_entry_t *new)
{
	lockmgr(&head->lock, LK_EXCLUSIVE);
	__add_wait_queue(head, new);
	lockmgr(&head->lock, LK_RELEASE);
}

static inline void
__remove_wait_queue(wait_queue_head_t *wqh, wait_queue_entry_t *wqe)
{
	list_del(&wqe->entry);
}

static inline void
remove_wait_queue(wait_queue_head_t *head, wait_queue_entry_t *old)
{
	lockmgr(&head->lock, LK_EXCLUSIVE);
	__remove_wait_queue(head, old);
	lockmgr(&head->lock, LK_RELEASE);
}

void
prepare_to_wait(wait_queue_head_t *q, wait_queue_entry_t *wait, int state);
void
finish_wait(wait_queue_head_t *q, wait_queue_entry_t *wait);

#if defined(__OpenBSD__)
#define __wait_event_intr_timeout(wqh, condition, timo, prio)		\
({									\
	long ret = timo;						\
	do {								\
		int __error;						\
		unsigned long deadline;					\
									\
		KASSERT(!cold);						\
									\
		mtx_enter(&sch_mtx);					\
		deadline = jiffies + ret;				\
		__error = msleep(&wqh, &sch_mtx, prio, "drmweti", ret);	\
		ret = deadline - jiffies;				\
		if (__error == ERESTART || __error == EINTR) {		\
			ret = -ERESTARTSYS;				\
			mtx_leave(&sch_mtx);				\
			break;						\
		}							\
		if ((timo) > 0 && (ret <= 0 || __error == EWOULDBLOCK)) { \
			mtx_leave(&sch_mtx);				\
			ret = ((condition)) ? 1 : 0;			\
			break;						\
 		}							\
		mtx_leave(&sch_mtx);					\
	} while (ret > 0 && !(condition));				\
	ret;								\
})
#else
/*
 * wait_event_interruptible_timeout:
 * - The process is put to sleep until the condition evaluates to true.
 * - The condition is checked each time the waitqueue wq is woken up.
 * - wake_up has to be called after changing any variable that could change
 * the result of the wait condition.
 *
 * returns:
 *   - 0 if the timeout elapsed
 *   - the remaining jiffies if the condition evaluated to true before
 *   the timeout elapsed.
 *   - remaining jiffies are always at least 1
 *   - -ERESTARTSYS if interrupted by a signal (when PCATCH is set in flags)
*/
#define __wait_event_common(wq, condition, timeout_jiffies, flags,	\
			    locked)					\
({									\
	int start_jiffies, elapsed_jiffies, remaining_jiffies, ret;	\
	bool timeout_expired = false;					\
	bool interrupted = false;					\
	long retval;							\
	int state;							\
	DEFINE_WAIT(tmp_wq);						\
									\
	start_jiffies = ticks;						\
	state = (flags & PCATCH) ? TASK_INTERRUPTIBLE : TASK_UNINTERRUPTIBLE; \
	prepare_to_wait(&wq, &tmp_wq, state);				\
									\
	while (1) {							\
		__wait_event_prefix(&wq, flags);			\
									\
		if (condition)						\
			break;						\
									\
		tsleep_interlock(current, flags);			\
									\
		if ((timeout_jiffies) != 0) {				\
			ret = tsleep(current, PINTERLOCKED|flags, "lwe", timeout_jiffies);	\
		} else {						\
			ret = tsleep(current, PINTERLOCKED|flags, "lwe", hz);\
			if (ret == EWOULDBLOCK) {			\
				/*kprintf("F");*/			\
				/*print_backtrace(-1);*/		\
				ret = 0;				\
			}						\
		}							\
									\
		if (ret == EINTR || ret == ERESTART) {			\
			interrupted = true;				\
			break;						\
		}							\
		if (ret == EWOULDBLOCK) {				\
			timeout_expired = true;				\
			break;						\
		}							\
	}								\
									\
	elapsed_jiffies = ticks - start_jiffies;			\
	remaining_jiffies = timeout_jiffies - elapsed_jiffies;		\
	if (remaining_jiffies <= 0)					\
		remaining_jiffies = 1;					\
									\
	if (timeout_expired)						\
		retval = 0;						\
	else if (interrupted)						\
		retval = -ERESTARTSYS;					\
	else if (timeout_jiffies > 0)					\
		retval = remaining_jiffies;				\
	else								\
		retval = 1;						\
									\
	finish_wait(&wq, &tmp_wq);					\
	retval;								\
})
#endif

#if defined(__OpenBSD__)
/*
 * Sleep until `condition' gets true.
 */
#define wait_event(wqh, condition) 		\
do {						\
	if (!(condition))			\
		__wait_event_intr_timeout(wqh, condition, 0, 0); \
} while (0)
#else
#define wait_event(wq, condition)					\
		__wait_event_common(wq, condition, 0, 0, false)
#endif

#if defined(__OpenBSD__)
#define wait_event_killable(wqh, condition) 		\
({						\
	int __ret = 0;				\
	if (!(condition))			\
		__ret = __wait_event_intr_timeout(wqh, condition, 0, PCATCH); \
	__ret;					\
})
#else
/* Not sure */
#define wait_event_killable(wqh, condition)				\
({									\
	long retval;							\
									\
	retval = __wait_event_common(wqh, condition, 0, PCATCH, false);	\
	if (retval != -ERESTARTSYS)					\
		retval = 0;						\
	retval;								\
})
#endif

#if defined(__OpenBSD__)
#define wait_event_interruptible(wqh, condition) 		\
({						\
	int __ret = 0;				\
	if (!(condition))			\
		__ret = __wait_event_intr_timeout(wqh, condition, 0, PCATCH); \
	__ret;					\
})
#else
#define wait_event_interruptible(wqh, condition)				\
({									\
	long retval;							\
									\
	retval = __wait_event_common(wqh, condition, 0, PCATCH, false);	\
	if (retval != -ERESTARTSYS)					\
		retval = 0;						\
	retval;								\
})
#endif

#if defined(__OpenBSD__)
#define wait_event_interruptible_locked(wqh, condition) 		\
({						\
	int __ret = 0;				\
	if (!(condition))			\
		__ret = __wait_event_intr_timeout(wqh, condition, 0, PCATCH); \
	__ret;					\
})
#else
#define wait_event_interruptible_locked(wqh, condition)			\
({									\
	long retval;							\
									\
	retval = __wait_event_common(wqh, condition, 0, PCATCH, true);	\
	if (retval != -ERESTARTSYS)					\
		retval = 0;						\
	retval;								\
})
#endif

#if defined(__OpenBSD__)
/*
 * Sleep until `condition' gets true or `timo' expires.
 *
 * Returns 0 if `condition' is still false when `timo' expires or
 * the remaining (>=1) jiffies otherwise.
 */
#define wait_event_timeout(wqh, condition, timo)	\
({						\
	long __ret = timo;			\
	if (!(condition))			\
		__ret = __wait_event_intr_timeout(wqh, condition, timo, 0); \
	__ret;					\
})
#else
#define wait_event_timeout(wqh, condition, timo)			\
		__wait_event_common(wqh, condition, timo, 0, false)
#endif

#if defined(__OpenBSD__)
/*
 * Sleep until `condition' gets true, `timo' expires or the process
 * receives a signal.
 *
 * Returns -ERESTARTSYS if interrupted by a signal.
 * Returns 0 if `condition' is still false when `timo' expires or
 * the remaining (>=1) jiffies otherwise.
 */
#define wait_event_interruptible_timeout(wqh, condition, timo) \
({						\
	long __ret = timo;			\
	if (!(condition))			\
		__ret = __wait_event_intr_timeout(wqh, condition, timo, PCATCH);\
	__ret;					\
})
#else
#define wait_event_interruptible_timeout(wqh, condition, timo)	\
		__wait_event_common(wqh, condition, timo, PCATCH, false)
#endif

// void init_wait_entry(struct wait_queue_entry *wq_entry, int flags);

// void __init_waitqueue_head(wait_queue_head_t *q, const char *name, struct lock_class_key *);

#if 0
static inline void
init_waitqueue_head(wait_queue_head_t *q)
{
	__init_waitqueue_head(q, "", NULL);
}
#endif

void __wake_up_core(wait_queue_head_t *q, int num_to_wake_up);
#if 0 && defined(__DragonFly__) /* From linux_wait.c */
void
__wake_up_core(wait_queue_head_t *wqh, int num_to_wake_up)
{
	wait_queue_entry_t *curr, *next;
	int mode = TASK_NORMAL;

	list_for_each_entry_safe(curr, next, &wqh->head, entry) {
		if (curr->func(curr, mode, 0, NULL))
			num_to_wake_up--;

		if (num_to_wake_up == 0)
			break;
	}
}
#endif

#if defined(__OpenBSD__)
static inline void
wake_up(wait_queue_head_t *wqh)
{
	wait_queue_entry_t *wqe;
	wait_queue_entry_t *tmp;
	mtx_enter(&wqh->lock);
	
	list_for_each_entry_safe(wqe, tmp, &wqh->head, entry) {
		KASSERT(wqe->func != NULL);
		if (wqe->func != NULL)
			wqe->func(wqe, 0, wqe->flags, NULL);
	}
	wakeup(wqh);
	mtx_leave(&wqh->lock);
}
#else
static inline void
wake_up(wait_queue_head_t *wqh)
{
	lockmgr(&wqh->lock, LK_EXCLUSIVE);
	__wake_up_core(wqh, 1);
	lockmgr(&wqh->lock, LK_RELEASE);
	wakeup_one(wqh);
}
#endif

#if defined(__OpenBSD__)
#define wake_up_all(wqh)			wake_up(wqh)
#else
static inline void
wake_up_all(wait_queue_head_t *wqh)
{
	lockmgr(&wqh->lock, LK_EXCLUSIVE);
	__wake_up_core(wqh, 0);
	lockmgr(&wqh->lock, LK_RELEASE);
	wakeup(wqh);
}
#endif

// void wake_up_bit(void *, int);

#if defined(__OpenBSD__)
static inline void
wake_up_all_locked(wait_queue_head_t *wqh)
{
	wait_queue_entry_t *wqe;
	wait_queue_entry_t *tmp;

	list_for_each_entry_safe(wqe, tmp, &wqh->head, entry) {
		KASSERT(wqe->func != NULL);
		if (wqe->func != NULL)
			wqe->func(wqe, 0, wqe->flags, NULL);
	}
	wakeup(wqh);
}
#else
#define wake_up_all_locked(wqh)		__wake_up_core(wqh, 0)
#endif

#define wake_up_interruptible(wqh)		wake_up(wqh)
#define wake_up_interruptible_poll(wqh, flags)	wake_up(wqh)
#define wake_up_interruptible_all(wqh)		wake_up_all(wqh)

void __wait_event_prefix(wait_queue_head_t *wq, int flags);

static inline int
waitqueue_active(wait_queue_head_t *q)
{
	return !list_empty(&q->head);
}

#if defined(__OpenBSD__)
#define	DEFINE_WAIT(name)				\
	struct wait_queue_entry name = {		\
		.private = curproc,			\
		.func = autoremove_wake_function,	\
		.entry = LIST_HEAD_INIT((name).entry),	\
	}
#else
#define DEFINE_WAIT_FUNC(name, _function)			\
	struct wait_queue_entry name = {			\
		.private = current,				\
		.func = _function,				\
		.entry = LIST_HEAD_INIT((name).entry),		\
	}

#define DEFINE_WAIT(name)	\
	DEFINE_WAIT_FUNC((name), autoremove_wake_function)
#endif

#define DECLARE_WAIT_QUEUE_HEAD(name)					\
	wait_queue_head_t name = {					\
		.lock = LOCK_INITIALIZER("name", 0, LK_CANRECURSE),	\
		.head = { &(name).head, &(name).head }	\
	}

#if defined(__OpenBSD)
static inline void
prepare_to_wait(wait_queue_head_t *wqh, wait_queue_entry_t *wqe, int state)
{
	if (wqe->flags == 0) {
		mtx_enter(&sch_mtx);
		wqe->flags = 1;
	}
	MUTEX_ASSERT_LOCKED(&sch_mtx);
	if (list_empty(&wqe->entry))
		__add_wait_queue(wqh, wqe);
	sch_proc = curproc;
	sch_ident = wqe;
	sch_priority = state;
}
#elif 0 && defined(__DragonFly__) /* linux_wait.c */
static inline void
prepare_to_wait(wait_queue_head_t *wqh, wait_queue_entry_t *wqe, int state)
{
	lockmgr(&wqh->lock, LK_EXCLUSIVE);
	if (list_empty(&wqe->entry))
		__add_wait_queue(wqh, wqe);
	current->state = (state);
	mb();
	lockmgr(&wqh->lock, LK_RELEASE);
}
#endif

#if defined(__OpenBSD)
static inline void
finish_wait(wait_queue_head_t *wqh, wait_queue_entry_t *wqe)
{
	MUTEX_ASSERT_LOCKED(&sch_mtx);
	sch_ident = NULL;
	if (!list_empty(&wqe->entry))
		list_del_init(&wqe->entry);
	mtx_leave(&sch_mtx);
}
#elif 0 && defined(__DragonFly__) /* linux_wait.c */
void
finish_wait(wait_queue_head_t *wqh, wait_queue_entry_t *wqe)
{
	set_current_state(TASK_RUNNING);

	lockmgr(&wqh->lock, LK_EXCLUSIVE);
	if (!list_empty(&wqe->entry))
		list_del_init(&wqe->entry);
	lockmgr(&wqh->lock, LK_RELEASE);
}
#endif

#endif

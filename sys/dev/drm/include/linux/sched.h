/*	$OpenBSD: sched.h,v 1.4 2021/07/07 02:38:36 jsg Exp $	*/
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
 * Copyright (c) 2019-2020 Matthew Dillon <dillon@backplane.com>
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

#ifndef	_LINUX_SCHED_H
#define	_LINUX_SCHED_H

#include <sys/param.h>
#include <sys/systm.h>
#if defined(__OpenBSD__)
#include <sys/kernel.h>
#endif
#include <sys/stdint.h>
#if defined(__OpenBSD__)
#include <sys/mutex.h>
#else
#include <sys/lock.h>
#endif

#include <sys/thread.h>
#include <sys/proc.h>
#include <sys/sched.h>   /* for struct sched_param */
#include <sys/signal2.h>

#include <machine/cpu.h> /* for any_resched_wanted() */

#if defined(__OpenBSD__)
#include <linux/wait.h>
#include <linux/sem.h>
#endif

#include <linux/atomic.h>
#include <linux/capability.h>
#include <linux/threads.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/rbtree.h>
#include <linux/thread_info.h>
#include <linux/cpumask.h>
#include <linux/errno.h>
#include <linux/mm_types.h>
#include <linux/preempt.h>

// #include <asm/page.h>

#include <linux/smp.h>
#include <linux/compiler.h>
#include <linux/completion.h>
#include <linux/pid.h>
#include <linux/rcupdate.h>
#include <linux/rculist.h>

#include <linux/time.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/llist.h>
#include <linux/gfp.h>

#include <asm/processor.h>

#include <linux/spinlock.h>

struct seq_file;

#if defined(__OpenBSD__)
#define TASK_NORMAL		1
#define TASK_UNINTERRUPTIBLE	0
#define TASK_INTERRUPTIBLE	PCATCH
#define TASK_RUNNING		-1
#else
#define	TASK_RUNNING		0
#define	TASK_INTERRUPTIBLE	1
#define	TASK_UNINTERRUPTIBLE	2

#define TASK_NORMAL		(TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE)
#endif

#if defined(__OpenBSD__)
#define MAX_SCHEDULE_TIMEOUT	(INT32_MAX)
#else
#define MAX_SCHEDULE_TIMEOUT    (LONG_MAX)
#endif

#define TASK_COMM_LEN		(MAXCOMLEN + 1)
#if 0 /* previous DragonFly */
#define TASK_COMM_LEN	MAXCOMLEN
#endif

struct task_struct {
	struct thread *dfly_td;
	volatile long state;
	struct mm_struct *mm;	/* mirror copy in p->p_linux_mm */
	int prio;

	/* kthread-specific data */
	unsigned long		kt_flags;
	int			(*kt_fn)(void *data);
	void			*kt_fndata;
	int			kt_exitvalue;

	/* executable name without path */
	char			comm[TASK_COMM_LEN];

	atomic_t usage_counter;
	pid_t pid;
	struct lock		kt_spin;
};

#if defined(__OpenBSD__)
#define cond_resched()		sched_pause(yield)
#else
/* Explicit rescheduling in order to reduce latency */
static inline int
cond_resched(void)
{
	lwkt_yield();
	return 0;
}
#endif

#if defined(__OpenBSD__)
#define drm_need_resched() \
    (curcpu()->ci_schedstate.spc_schedflags & SPCF_SHOULDYIELD)
#else /* not sure */
#define drm_need_resched() \
    lwkt_resched_wanted() 
#endif

static inline int
#if defined(__OpenBSD__)
cond_resched_lock(struct mutex *mtxp)
#else
cond_resched_lock(struct lock *mtxp)
#endif
{
	if (drm_need_resched() == 0)
		return 0;

	lockmgr(mtxp, LK_RELEASE);
	cond_resched();
	lockmgr(mtxp, LK_EXCLUSIVE);

	return 1;
}

#if defined(__OpenBSD__)
void set_current_state(int);
void __set_current_state(int);
#endif

#define __set_current_state(state_value)	current->state = (state_value);

#define set_current_state(state_value)		\
do {						\
	__set_current_state(state_value);	\
	mb();					\
} while (0)

#if defined(__OpenBSD__)
void schedule(void);
long schedule_timeout(long);
long schedule_timeout_uninterruptible(long);
#endif

/*
 * schedule_timeout: puts the current thread to sleep until timeout
 * if its state allows it to.
 */
static inline long
schedule_timeout(signed long timeout)
{
	unsigned long time_before, time_after;
	long slept, ret = 0;
	int timo;

	if (timeout < 0) {
		kprintf("schedule_timeout(): timeout cannot be negative\n");
		goto done;
	}

	/*
	 * Indefinite wait if timeout is MAX_SCHEDULE_TIMEOUT, but we are
	 * also translating to an integer.  The first conditional will
	 * cover both but to code defensively test both.
	 */
	if (timeout >= INT_MAX || timeout == MAX_SCHEDULE_TIMEOUT)
		timo = 0;
	else
		timo = timeout;

	lockmgr(&current->kt_spin, LK_EXCLUSIVE);

	switch (current->state) {
	case TASK_INTERRUPTIBLE:
		time_before = ticks;
		lksleep(current, &current->kt_spin, PCATCH, "lstim", timo);
		time_after = ticks;
		slept = time_after - time_before;
		ret = timeout - slept;
		if (ret < 0)
			ret = 0;
		break;
	case TASK_UNINTERRUPTIBLE:
		lksleep(current, &current->kt_spin, 0, "lstim", timo);
		break;
	default:
		/*
		 * Task has been flagged running before we could
		 * enter the sleep.
		 *
		 * XXX should be able to remove this ssleep(), have it
		 * here to protect against live-locks in case we mess
		 * up the task->state.
		 */
		lksleep(current, &current->kt_spin, 0, "lst1", 1);
		break;
	}

	lockmgr(&current->kt_spin, LK_RELEASE);

done:
	if (timeout == MAX_SCHEDULE_TIMEOUT)
		ret = MAX_SCHEDULE_TIMEOUT;

	current->state = TASK_RUNNING;
	return ret;
}

static inline void
schedule(void)
{
	(void)schedule_timeout(MAX_SCHEDULE_TIMEOUT);
}

static inline signed long
schedule_timeout_uninterruptible(signed long timeout)
{
	__set_current_state(TASK_UNINTERRUPTIBLE);
	return schedule_timeout(timeout);
}

#define io_schedule_timeout(x)	schedule_timeout(x)
#if 0 /* previous DragonFly */
static inline long
io_schedule_timeout(signed long timeout)
{
	return schedule_timeout(timeout);
}
#endif

#if defined(__OpenBSD__)
struct proc;
int wake_up_process(struct proc *p);
#else
static inline int
wake_up_process(struct task_struct *tsk)
{
	long ostate;

	/*
	 * Among other things, this function is supposed to act as
	 * a barrier
	 */
	smp_wmb();
	lockmgr(&tsk->kt_spin, LK_EXCLUSIVE);
	ostate = tsk->state;
	tsk->state = TASK_RUNNING;
	lockmgr(&tsk->kt_spin, LK_RELEASE);
	/* if (ostate != TASK_RUNNING) */
	wakeup(tsk);

	return 1;	/* Always indicate the process was woken up */
}
#endif

#if 0
/*
 * local_clock: fast time source, monotonic on the same cpu
 */
static inline uint64_t
local_clock(void)
{
	struct timespec ts;

	getnanouptime(&ts);
	return (ts.tv_sec * NSEC_PER_SEC) + ts.tv_nsec;
}
#endif

static inline void
yield(void)
{
	lwkt_yield();
}

#if 0
static inline int
signal_pending(struct task_struct *p)
{
	struct thread *t = p->dfly_td;

	/* Some kernel threads do not have lwp, t->td_lwp can be NULL */
	if (t->td_lwp == NULL)
		return 0;

	return CURSIG(t->td_lwp);
}

static inline int
fatal_signal_pending(struct task_struct *p)
{
	struct thread *t = p->dfly_td;
	sigset_t pending_set;

	/* Some kernel threads do not have lwp, t->td_lwp can be NULL */
	if (t->td_lwp == NULL)
		return 0;

	pending_set = lwp_sigpend(t->td_lwp);
	return SIGISMEMBER(pending_set, SIGKILL);
}

static inline int
signal_pending_state(long state, struct task_struct *p)
{
	if (state & TASK_INTERRUPTIBLE)
		return (signal_pending(p));
	else
		return (fatal_signal_pending(p));
}
#endif


#if 0
static inline int
send_sig(int sig, struct proc *p, int priv)
{
	ksignal(p, sig);
	return 0;
}
#endif

#if 0 /* not used in kernel drm or apparently anywhere else now */
static inline void
set_need_resched(void)
{
	/* do nothing for now */
	/* used on ttm_bo_reserve failures */
}
#endif

/* need_resched(struct cpu_info *) OpenBSD */
static inline bool
need_resched(void)
{
	return any_resched_wanted();
}

/* no longer used by kernel drm 5.15 */
static inline int
sched_setscheduler_nocheck(struct task_struct *ts,
			   int policy, const struct sched_param *param)
{
	/* We do not allow different thread scheduling policies */
	return 0;
}

#if 0
static inline int
pagefault_disabled(void)
{
	return (curthread->td_flags & TDF_NOFAULT);
}
#endif

static inline void
mmgrab(struct mm_struct *mm)
{
	atomic_inc(&mm->mm_count);
}

#endif

/*
 * Copyright (c) 2019 François Tigeot <ftigeot@wolfpond.org>
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

#include <sys/lock.h>
#include <sys/kthread.h>

#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/slab.h>

#if defined(__OpenBSD__)
struct kthread {
	int (*func)(void *);
	void *data;
	struct proc *proc;
	volatile u_int flags;
#define KTHREAD_SHOULDSTOP	0x0000001
#define KTHREAD_STOPPED		0x0000002
#define KTHREAD_SHOULDPARK	0x0000004
#define KTHREAD_PARKED		0x0000008
	LIST_ENTRY(kthread) next;
};

LIST_HEAD(, kthread) kthread_list = LIST_HEAD_INITIALIZER(kthread_list);
#endif /* OpenBSD */

/*
   All Linux threads/processes have an associated task_struct
   a kthread is a pure kernel thread without userland context
*/

static void
kthread_func(void *arg)
{
#if defined(__OpenBSD__)
	struct kthread *thread = arg;
	int ret;

	ret = thread->func(thread->data);
	thread->flags |= KTHREAD_STOPPED;
	wakeup(thread);
	kthread_exit(ret);
#else
	struct task_struct *task = arg;

	task->kt_exitvalue = task->kt_fn(task->kt_fndata);
#endif
}

struct task_struct *
kthread_run(int (*func)(void *), void *data, const char *name, ...)
{
#if defined(__OpenBSD__)
	struct kthread *thread;

	thread = malloc(sizeof(*thread), M_DRM, M_WAITOK);
	thread->func = func;
	thread->data = data;
	thread->flags = 0;
	
	if (kthread_create(kthread_func, thread, &thread->proc, name)) {
		free(thread, M_DRM, sizeof(*thread));
		return ERR_PTR(-ENOMEM);
	}

	LIST_INSERT_HEAD(&kthread_list, thread, next);
	return thread->proc;
#else
	struct task_struct *task;
	struct thread *td;
	__va_list args;
	int ret;

	task = kzalloc(sizeof(*task), GFP_KERNEL);

	__va_start(args, name);
	ret = kthread_alloc(kthread_func, task, &td, name, args);
	__va_end(args);
	if (ret) {
		kfree(task);
		return ERR_PTR(-ENOMEM);
	}

	task->dfly_td = td;
	td->td_linux_task = task;

	task->mm = NULL;	/* kthreads have no userland address space */

	task->kt_fn = func;
	task->kt_fndata = data;
	lockinit(&task->kt_spin, "tspin1", 0, 0);

	/* Start the thread here */
	lwkt_schedule(td);

	return task;
#endif
}

#define KTHREAD_SHOULD_STOP 1
#define KTHREAD_SHOULD_PARK 2

bool
kthread_should_stop(void)
{
	return test_bit(KTHREAD_SHOULD_STOP, &current->kt_flags);
}

int
kthread_stop(struct task_struct *ts)
{
	set_bit(KTHREAD_SHOULD_STOP, &ts->kt_flags);

	kthread_unpark(ts);
	wake_up_process(ts);

	/* XXX use a better mechanism to wait for the thread to finish running */
	tsleep(kthread_stop, 0, "kstop", hz);
	lwkt_free_thread(ts->dfly_td);

	return ts->kt_exitvalue;
}

int
kthread_park(struct task_struct *ts)
{
	set_bit(KTHREAD_SHOULD_PARK, &ts->kt_flags);
	wake_up_process(ts);

	return ts->kt_exitvalue;
}

void
kthread_unpark(struct task_struct *ts)
{
	clear_bit(KTHREAD_SHOULD_PARK, &ts->kt_flags);
	lwkt_schedule(ts->dfly_td);
	wake_up_process(ts);
}

bool
kthread_should_park(void)
{
	return test_bit(KTHREAD_SHOULD_PARK, &current->kt_flags);
}

void
kthread_parkme(void)
{
	if (test_bit(KTHREAD_SHOULD_PARK, &current->kt_flags) == 0)
		return;

	lwkt_deschedule_self(curthread);
}

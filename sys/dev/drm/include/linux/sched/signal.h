/* Public domain. */

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
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LINUX_SCHED_SIGNAL_H
#define _LINUX_SCHED_SIGNAL_H

#include <sys/param.h>
#include <sys/systm.h>
#if defined(__OpenBSD__)
#include <sys/signalvar.h>
#else
#include <sys/thread.h>
#include <sys/signal2.h>
#endif

// #include <linux/rculist.h>
#include <linux/sched.h> /* definition of struct task_struct */
// #include <linux/sched/task.h>

#if defined(__OpenBSD__)

#define signal_pending_state(s, x) \
    ((s) & TASK_INTERRUPTIBLE ? SIGPENDING(curproc) : 0)
#define signal_pending(y) SIGPENDING(curproc)

#else

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

static inline int
send_sig(int sig, struct proc *p, int priv)
{
	ksignal(p, sig);
	return 0;
}

#endif

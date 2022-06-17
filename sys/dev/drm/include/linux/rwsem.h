/* Public domain. */

/*
 * Copyright (c) 2018-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_RWSEM_H
#define _LINUX_RWSEM_H

#if 0
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#endif

#include <sys/lock.h>

#if defined(__OpenBSD__)
#define down_read(rwl)			rw_enter_read(rwl)
#else
#define down_read(rwl)			lockmgr((rwl), LK_SHARED)
#endif

#if defined(__OpenBSD__)
#define down_read_trylock(rwl)		(rw_enter(rwl, RW_READ | RW_NOSLEEP) == 0)
#else
/*
 * trylock for reading -- returns 1 if successful, 0 if contention
 */
static inline int
down_read_trylock(struct lock *rwl) {
	return !lockmgr(rwl, LK_EXCLUSIVE | LK_NOWAIT);
}
#endif

#if defined(__OpenBSD__)
#define down_write_trylock(rwl)		(rw_enter(rwl, RW_WRITE | RW_NOSLEEP) == 0)
#else /* not sure */
#define down_write_trylock(rwl)		(!lockmgr(rwl, LK_EXCLUSIVE | LK_NOWAIT))
#endif

#if defined(__OpenBSD__)
#define up_read(rwl)			rw_exit_read(rwl)
#define down_write(rwl)			rw_enter_write(rwl)
#else
#define up_read(rwl)			lockmgr((rwl), LK_RELEASE)
#define down_write(rwl)			lockmgr((rwl), LK_EXCLUSIVE)
#endif

#if defined(__OpenBSD__)
#define down_write_nest_lock(rwl, x)	rw_enter_write(rwl)
#else /* not sure */
#define down_write_nest_lock(rwl, x)	lockmgr((rwl), LK_EXCLUSIVE)
#endif

#if defined(__OpenBSD__)
#define up_write(rwl)			rw_exit_write(rwl)
#else
#define up_write(rwl)			lockmgr((rwl), LK_RELEASE)
#endif

#if defined(__OpenBSD__)
#define downgrade_write(rwl)		rw_enter(rwl, RW_DOWNGRADE)
#else
/* DragonFly commented out in radeon_device.c */
#endif

static inline int
down_write_killable(struct lock *rwl)
{
	if (lockmgr(rwl, LK_EXCLUSIVE | LK_SLEEPFAIL | LK_PCATCH))
		return -EINTR;

	return 0;
}

#endif

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

#ifndef _LINUX_STOP_MACHINE_H
#define _LINUX_STOP_MACHINE_H

#if 0
#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/list.h>

#include <sys/mplock2.h>
#endif

#if defined(__OpenBSD__)
#include <machine/intr.h>
#else
#include <sys/types.h>
#include <machine/cpufunc.h>
#endif

typedef int (*cpu_stop_fn_t)(void *arg);

static inline int
stop_machine(cpu_stop_fn_t fn, void *arg, void *cpus)
{

#if 0
	/* XXX: is this enough ?
	 * See Linux commit 5bab6f60cb4d1417ad7c599166bcfec87529c1a2 */
       int r;
       get_mplock();
       r = (*fn)(arg);
       rel_mplock();
       return r;
#else
	int r;
	u_long s = intr_disable();
	r = (*fn)(arg);
	intr_restore(s);
	return r;
#endif
}

#endif

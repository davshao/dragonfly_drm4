/* Public domain. */

/*
 * Copyright (c) 2019-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_SMP_H
#define _LINUX_SMP_H

#if defined(__OpenBSD__)
#include <linux/cpumask.h>

/* sparc64 cpu.h needs time.h and siginfo.h (indirect via param.h) */
#include <sys/param.h>
#include <machine/cpu.h>
#else
#include <machine/cpufunc.h>
#endif

// #include <linux/errno.h>
// #include <linux/types.h>
// #include <linux/list.h>
#include <linux/cpumask.h>
// #include <linux/init.h>
// #include <linux/llist.h>

#include <linux/preempt.h>
// #include <linux/kernel.h>
// #include <linux/compiler.h>
// #include <linux/thread_info.h>

#if defined(__OpenBSD__)
#define smp_processor_id()	(curcpu()->ci_cpuid)
#else
static inline uint32_t
smp_processor_id(void)
{
	return mycpuid;
}
#endif

#if defined(__OpenBSD__)

#define get_cpu()		cpu_number()
#define put_cpu()

#else

static inline uint32_t get_cpu(void)
{
	preempt_disable();

	/* Regular DragonFly kernel threads always run on the same CPU */
	return 0;
}

#define put_cpu()	preempt_enable()

#endif

#endif

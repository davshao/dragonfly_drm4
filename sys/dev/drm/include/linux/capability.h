/* Public domain. */

/*
 * Copyright (c) 2015 Rimvydas Jasinskas
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

/*
 * linux/capability.h
 *
 * Simple capable() priv_check helper
 *
 * Note enforced "const int" safety check
 */

#ifndef _LINUX_CAPABILITY_H
#define _LINUX_CAPABILITY_H

// #include <uapi/linux/capability.h>

// #include <sys/types.h>

#include <sys/param.h>
#include <sys/systm.h>
#if defined(__OpenBSD__)
#include <sys/ucred.h>
#include <machine/cpu.h>
#else
#include <sys/globaldata.h>
#include <sys/thread.h>
#include <sys/caps.h>
#endif

// #include <linux/types.h>

struct task_struct;

#if defined(__OpenBSD__)
#define CAP_SYS_ADMIN	0x1
#define CAP_SYS_NICE	0x2
#else
#define CAP_SYS_ADMIN		SYSCAP_RESTRICTEDROOT
#define CAP_SYS_NICE		SYSCAP_RESTRICTEDROOT
#endif

#if defined(__OpenBSD__)
static inline bool
capable(int cap) 
{ 
	switch (cap) {
	case CAP_SYS_ADMIN:
	case CAP_SYS_NICE:
		return suser(curproc) == 0;
	default:
		panic("unhandled capability");
	}
}
#else
static inline bool
capable(const int cap)
{
	return (caps_priv_check_self(cap) == 0);
}
#endif

static inline bool
perfmon_capable(void)
{
	return false;
}

#endif

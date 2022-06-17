/* Public domain. */

/*
 * Copyright (c) 2016-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _ASM_CPUFEATURE_H
#define _ASM_CPUFEATURE_H

#include <sys/systm.h>
#include <machine/specialreg.h>

// #include <asm/processor.h>

// #include <asm/cpufeatures.h>

#if defined(__OpenBSD__)
#define X86_FEATURE_CLFLUSH	1
#define X86_FEATURE_XMM4_1	2
#define X86_FEATURE_PAT		3
#define X86_FEATURE_HYPERVISOR	4
#else
#define X86_FEATURE_CLFLUSH	( 0*32+19) /* CLFLUSH instruction */
#define X86_FEATURE_XMM4_1	( 4*32+19) /* SSE 4.1, first appeared in 2007 */
#define X86_FEATURE_PAT		( 0*32+16)
#define X86_FEATURE_HYPERVISOR	( 4*32+31) /* Running on a hypervisor */
#endif

static inline bool
static_cpu_has(uint16_t f)
{
	switch (f) {
	case X86_FEATURE_CLFLUSH:
		/* All amd64 CPUs have the clflush instruction */
		return true;
	case X86_FEATURE_XMM4_1:
#if defined(__OpenBSD__)
		return (cpu_ecxfeature & CPUIDECX_SSE41) != 0;
#else
		return ((cpu_feature2 & CPUID2_SSE41) != 0);
#endif
	case X86_FEATURE_PAT:
		/* All amd64 CPUs have PAT support */
		return true;
	case X86_FEATURE_HYPERVISOR:
		return ((cpu_feature2 & CPUID2_VMM) != 0);
	default:
		return false;
	}
}

#define boot_cpu_has(x)	static_cpu_has(x)

#endif

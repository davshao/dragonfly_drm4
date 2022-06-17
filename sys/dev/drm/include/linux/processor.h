/* Public domain. */

#ifndef _LINUX_PROCESSOR_H
#define _LINUX_PROCESSOR_H

#include <sys/systm.h>

#if defined(__OpenBSD__)
/* sparc64 cpu.h needs time.h and siginfo.h (indirect via param.h) */
#include <sys/param.h>
#include <machine/cpu.h>
#else
#include <machine/cpufunc.h>
#endif

// #include <linux/jiffies.h>

#if defined(__OpenBSD__)
static inline void
cpu_relax(void)
{
	CPU_BUSY_CYCLE();
	if (cold) {
		delay(tick);
		jiffies++;
	}
}
#else
#define cpu_relax()	cpu_pause()
#endif

// #include <asm/processor.h>

#ifndef CACHELINESIZE
#define CACHELINESIZE 64
#endif

#endif

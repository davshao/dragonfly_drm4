/* Public domain. */

/*
 * Copyright (c) 2014-2016 François Tigeot
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

#ifndef _LINUX_IO_H
#define _LINUX_IO_H

#include <sys/types.h>
#include <sys/systm.h>
/* on DragonFly sys/systm.h includes machine/cpufunc.h,
 * where readb, writeb etc. are already defined */

#if defined(__OpenBSD__)
#include <sys/memrange.h> /* for MDF_WRITECOMBINE */

#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/compiler.h>
#include <linux/vmalloc.h>
#else
#include <linux/kernel.h>
#include <linux/bug.h>
#include <asm/io.h>
#include <asm/page.h>
#endif

#if defined(__OpenBSD__)
#define memcpy_toio(d, s, n)	memcpy(d, s, n)
#define memcpy_fromio(d, s, n)	memcpy(d, s, n)
#define memset_io(d, b, n)	memset(d, b, n)
#else
/* XXX these should have volatile */
#define memcpy_toio(d, s, n)	memcpy((d), (s), (n))
#define memcpy_fromio(d, s, n)	memcpy((d), (s), (n))
#define memset_io(d, b, n)	memset((d), (b), (n))
#endif

#if defined(__OpenBSD__)
#ifdef __powerpc__
#define iobarrier()		mb()
#else
#define iobarrier()		barrier()
#endif
#endif

#if defined(__OpenBSD__)
static inline u8
ioread8(const volatile void __iomem *addr)
{
	uint8_t val;

	iobarrier();
	val = *(volatile uint8_t *)addr;
	rmb();
	return val;
}
#else
#define ioread8(addr)		*(volatile uint8_t *)((char *)addr)
#endif

#if defined(__OpenBSD__)
static inline void
iowrite8(u8 val, volatile void __iomem *addr)
{
	wmb();
	*(volatile uint8_t *)addr = val;
}
#else
#define iowrite8(val, addr)					\
	do {							\
		*(volatile uint8_t *)((char *)addr) = val;	\
	} while (0)
#endif

#if defined(__OpenBSD__)
static inline u16
ioread16(const volatile void __iomem *addr)
{
	uint16_t val;

	iobarrier();
	val = lemtoh16(addr);
	rmb();
	return val;
}
#else
#define ioread16(addr)		*(volatile uint16_t *)((char *)addr)
#endif

#if defined(__OpenBSD__)
static inline u32
ioread32(const volatile void __iomem *addr)
{
	uint32_t val;
	
	iobarrier();
	val = lemtoh32(addr);
	rmb();
	return val;
}
#else
#define ioread32(addr)		*(volatile uint32_t *)((char *)addr)
#endif

/* not sure */
#if defined(__OpenBSD__)
static inline u64
ioread64(const volatile void __iomem *addr)
{
	uint64_t val;

	iobarrier();
	val = lemtoh64(addr);
	rmb();
	return val;
}
#else
#define ioread64(addr)		*(volatile uint64_t *)((char *)addr)
#endif


#if defined(__OpenBSD__)
static inline void
iowrite16(u16 val, volatile void __iomem *addr)
{
	wmb();
	htolem16(addr, val);
}
#else
#define iowrite16(val, addr)					\
	do {							\
		*(volatile uint16_t *)((char *)addr) = val;	\
	} while (0)
#endif

#if defined(__OpenBSD__)
static inline void
iowrite32(u32 val, volatile void __iomem *addr)
{
	wmb();
	htolem32(addr, val);
}
#else
#define iowrite32(val, addr)					\
	do {							\
		*(volatile uint32_t *)((char *)addr) = val;	\
	} while (0)
#endif

#if defined(__OpenBSD__)
static inline void
iowrite64(u64 val, volatile void __iomem *addr)
{
	wmb();
	htolem64(addr, val);
}
#else
#define iowrite64(val, addr)					\
	do {							\
		*(volatile uint64_t *)((char *)addr) = val;	\
	} while (0)
#endif

#if defined(__OpenBSD__)
#define readb(p) ioread8(p)
#else
#undef readb
static inline u8
readb(const volatile void __iomem *addr)
{
	return *(const volatile u8*)addr;
}
#endif

#if defined(__OpenBSD__)
#define writeb(v, p) iowrite8(v, p)
#else
#undef writeb
static inline void
writeb(u8 value, volatile void __iomem *addr)
{
	*(volatile uint8_t *)addr = value;
}
#endif

#if defined(__OpenBSD__)
#define readw(p) ioread16(p)
#else
#undef readw
static inline u16
readw(const volatile void __iomem *addr)
{
	return *(const volatile u16*)addr;
}
#endif

#if defined(__OpenBSD__)
#define writew(v, p) iowrite16(v, p)
#else
#undef writew
static inline void
writew(u16 value, volatile void __iomem *addr)
{
	*(volatile uint16_t *)addr = value;
}
#endif

#if defined(__OpenBSD__)
#define readl(p) ioread32(p)
#else
#undef readl
static inline u32
readl(const volatile void __iomem *addr)
{
	return *(const volatile u32*)addr;
}
#endif

#if defined(__OpenBSD__)
#define writel(v, p) iowrite32(v, p)
#else
#undef writel
static inline void
writel(u32 value, volatile void __iomem *addr)
{
	*(volatile uint32_t *)addr = value;
}
#endif

#if defined(__OpenBSD__)
#define readq(p) ioread64(p)
#else
#undef readq
static inline u64
readq(const volatile void __iomem *addr)
{
	return *(const volatile u64*)addr;
}
#endif

#if defined(__OpenBSD__)
#define writeq(v, p) iowrite64(v, p)
#else
#undef writeq
static inline void
writeq(u64 value, volatile void __iomem *addr)
{
	*(volatile uint64_t *)addr = value;
}
#endif

#define writel_relaxed(v, a)	writel(v, a)

static inline int
arch_phys_wc_add(unsigned long base, unsigned long size)
{
	/* The DragonFly kernel is PAT-enabled, do nothing */
	return 0;
}

static inline void
arch_phys_wc_del(int handle)
{
	BUG_ON(handle != 0);
}

#endif

/* Public domain. */

/*
 * Copyright (c) 2015-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_GFP_H
#define _LINUX_GFP_H

#include <sys/types.h>
#include <sys/malloc.h>
#if defined(__OpenBSD__)
#include <uvm/uvm_extern.h>
#else
#include <vm/vm_page.h>
#include <machine/bus_dma.h>
#endif
#include <linux/mmzone.h>
// #include <linux/mmdebug.h>
// #include <linux/stddef.h>

#define __GFP_ZERO		M_ZERO
#if defined(__OpenBSD__)
#define __GFP_DMA32		0x00010000
#else
#define __GFP_DMA32		0x10000u	/* XXX: MUST NOT collide with the M_XXX definitions */
#endif
#define __GFP_NOWARN		0
#if defined(__OpenBSD__)
#define __GFP_NORETRY		0
#define __GFP_RETRY_MAYFAIL	0
#else
#define __GFP_NORETRY		M_NULLOK
#define __GFP_RETRY_MAYFAIL	M_NULLOK
#endif
#define __GFP_MOVABLE		0
#define __GFP_COMP		0
#define __GFP_KSWAPD_RECLAIM	0
#define __GFP_HIGHMEM		0
#define __GFP_RECLAIMABLE	0
#define __GFP_NOMEMALLOC	0

#define __GFP_RECLAIM		0
#define __GFP_NOFAIL		0
#define __GFP_IO		0

#if defined(__OpenBSD__)
#define GFP_ATOMIC		M_NOWAIT
#define GFP_NOWAIT		M_NOWAIT
#define GFP_KERNEL		(M_WAITOK | M_CANFAIL)
#define GFP_USER		(M_WAITOK | M_CANFAIL)
#define GFP_HIGHUSER		0
#else
#define GFP_ATOMIC		(M_NOWAIT | M_CACHEALIGN)
#define GFP_NOWAIT		(M_NOWAIT | M_CACHEALIGN)
#define GFP_KERNEL		(M_WAITOK | M_CACHEALIGN)
#define GFP_USER		GFP_KERNEL
#define GFP_HIGHUSER		GFP_KERNEL
#endif
#define GFP_DMA32		__GFP_DMA32
#define GFP_TRANSHUGE_LIGHT	0

#define GFP_TEMPORARY		GFP_KERNEL

static inline bool
gfpflags_allow_blocking(const unsigned int flags)
{
#if defined(__OpenBSD__) || defined(__DragonFly__)
	return (flags & M_WAITOK) != 0;
#else
	return (flags & M_WAITOK);
#endif
}

#if defined(__OpenBSD__)
struct vm_page *alloc_pages(unsigned int, unsigned int);
#else
static inline struct page *
#endif
alloc_pages(unsigned int gfp_mask, unsigned int order)
{
#if defined(__OpenBSD__) /* drm_linux.c */
	int flags = (gfp_mask & M_NOWAIT) ? UVM_PLA_NOWAIT : UVM_PLA_WAITOK;
	struct uvm_constraint_range *constraint = &no_constraint;
	struct pglist mlist;

	if (gfp_mask & M_CANFAIL)
		flags |= UVM_PLA_FAILOK;
	if (gfp_mask & M_ZERO)
		flags |= UVM_PLA_ZERO;
	if (gfp_mask & __GFP_DMA32)
		constraint = &dma_constraint;

	TAILQ_INIT(&mlist);
	if (uvm_pglistalloc(PAGE_SIZE << order, constraint->ucr_low,
	    constraint->ucr_high, PAGE_SIZE, 0, &mlist, 1, flags))
		return NULL;
	return TAILQ_FIRST(&mlist);
#else
/*
 * Allocate multiple contiguous pages. The DragonFly code can only do
 * multiple allocations via the free page reserve.  Linux does not appear
 * to restrict the address space, so neither do we.
 */
	size_t bytes = PAGE_SIZE << order;
	struct vm_page *pgs;

	pgs = vm_page_alloc_contig(0LLU, ~0LLU, bytes, bytes, bytes,
				   VM_MEMATTR_DEFAULT);

	return (struct page*)pgs;
#endif
}

#if defined(__OpenBSD__)
void	__free_pages(struct vm_page *, unsigned int);
#else
static inline
void	__free_pages(struct page *pgs, unsigned int order)
#endif
{
#if defined(__OpenBSD__)
	struct pglist mlist;
	int i;
	
	TAILQ_INIT(&mlist);
	for (i = 0; i < (1 << order); i++)
		TAILQ_INSERT_TAIL(&mlist, &page[i], pageq);
	uvm_pglistfree(&mlist);
#else
/*
 * Free multiple contiguous pages
 */
	size_t bytes = PAGE_SIZE << order;

	vm_page_free_contig((struct vm_page *)pgs, bytes);
#endif
}

#if defined(__OpenBSD__)
static inline struct vm_page *
#else
static inline struct page *
#endif
alloc_page(unsigned int gfp_mask)
{
#if defined(__OpenBSD__)
	return alloc_pages(gfp_mask, 0);
#else
	vm_paddr_t high = ~0LLU;

	if (gfp_mask & GFP_DMA32)
		high = BUS_SPACE_MAXADDR_32BIT;

	return (struct page *)vm_page_alloc_contig(0LLU, ~0LLU,
			PAGE_SIZE, PAGE_SIZE, PAGE_SIZE,
			VM_MEMATTR_DEFAULT);
#endif
}


static inline void
#if defined(__OpenBSD__)
__free_page(struct vm_page *page)
#else
__free_page(struct page *page)
#endif
{
#if defined(__OpenBSD__)
	__free_pages(page, 0);
#else
	vm_page_free_contig((struct vm_page *)page, PAGE_SIZE);
#endif
}

/* need to implement for DragonFly */
#if defined(__OpenBSD__)
/* Only used in notyet code i915_gpu_error.c or in a selftest_lrc.c */
/* Also for 6.1 in i915/gt/uc/intel_guc_log.c */
static inline unsigned long
__get_free_page(unsigned int gfp_mask)
{
	void *addr = km_alloc(PAGE_SIZE, &kv_page, &kp_dirty, &kd_nowait);
	return (unsigned long)addr;
}

static inline void
free_page(unsigned long addr)
{
	km_free((void *)addr, PAGE_SIZE, &kv_page, &kp_dirty);
}
#endif

#endif

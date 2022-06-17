/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013, 2014 Mellanox Technologies, Ltd.
 * Copyright (c) 2015 Matthew Dillon <dillon@backplane.com>
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
#ifndef	_LINUX_MM_H
#define	_LINUX_MM_H

#include <sys/types.h>
#include <sys/param.h>

#if defined(__OpenBSD__)
#include <sys/atomic.h>
#include <machine/cpu.h>
#include <uvm/uvm_extern.h>
#include <uvm/uvm_glue.h>
#else
#include <vm/vm_page.h>
#include <linux/atomic.h>
#endif

// #include <linux/errno.h>

#include <linux/fs.h>
#include <linux/shrinker.h>
#include <linux/overflow.h>
#include <asm/pgtable.h>

#if 0
#include <linux/mmdebug.h>
#include <linux/gfp.h>
#include <linux/bug.h>
#include <linux/list.h>
#include <linux/mmzone.h>
#include <linux/rbtree.h>
#endif
#include <linux/mm_types.h>
#if 0
#include <linux/err.h>
#include <asm/page.h>
#include <asm/processor.h>
#endif

struct page;

#define PageHighMem(x)	0

#if defined(__OpenBSD__)
#define page_to_phys(page)	(VM_PAGE_TO_PHYS(page))
#else
static inline vm_paddr_t
page_to_phys(struct page *page)
{
	struct vm_page *p = (struct vm_page *)page;

	return VM_PAGE_TO_PHYS(p);
}
#endif

#include <vm/vm_object.h>

#if defined(__OpenBSD__)
#define page_to_pfn(pp)		(VM_PAGE_TO_PHYS(pp) / PAGE_SIZE)
#else
static inline unsigned long
page_to_pfn(struct page *page)
{
	struct vm_page *p = (struct vm_page *)page;

	return OFF_TO_IDX(VM_PAGE_TO_PHYS(p));
}
#endif

#if defined(__OpenBSD__)
#define pfn_to_page(pfn)	(PHYS_TO_VM_PAGE(ptoa(pfn)))
#else
static inline struct page *
pfn_to_page(unsigned long pfn)
{
	return (struct page *)PHYS_TO_VM_PAGE(pfn << PAGE_SHIFT);
}
#endif

#if defined(__OpenBSD__)
#define nth_page(page, n)	(&(page)[(n)])
#else
static inline struct page *
nth_page(struct page *page, int n)
{
	return page + n;
}
#endif

#if defined(__OpenBSD__)
#define offset_in_page(off)	((vaddr_t)(off) & PAGE_MASK)
#else
#define offset_in_page(off)	((unsigned long)(off) & PAGE_MASK)
#endif

#if defined(__OpenBSD__)
#define set_page_dirty(page)	atomic_clearbits_int(&page->pg_flags, PG_CLEAN)
#else
static inline void
set_page_dirty(struct page *page)
{
	vm_page_dirty((struct vm_page *)page);
}
#endif

#if defined(__OpenBSD__)
#define PAGE_ALIGN(addr)	(((addr) + PAGE_MASK) & ~PAGE_MASK)
#else
#define PAGE_ALIGN(addr)	round_page(addr)
#endif

#define PFN_UP(x)		(((x) + PAGE_SIZE-1) >> PAGE_SHIFT)
#define PFN_DOWN(x)		((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)		((x) << PAGE_SHIFT)

#if defined(__OpenBSD__)
bool is_vmalloc_addr(const void *);
#else
int is_vmalloc_addr(const void *x);
#endif

#if defined(__OpenBSD__)
static inline void *
kvmalloc(size_t size, gfp_t flags)
{
	return malloc(size, M_DRM, flags);
}
#else
void *kvmalloc(size_t size, gfp_t flags);
#endif

#if defined(__OpenBSD__)
static inline void *
kvmalloc_array(size_t n, size_t size, int flags)
{
	if (n != 0 && SIZE_MAX / n < size)
		return NULL;
	return malloc(n * size, M_DRM, flags);
}
#else
void *
kvmalloc_array(size_t n, size_t size, int flags);
#endif

#if defined(__OpenBSD__)
static inline struct vm_page *
vmalloc_to_page(const void *va)
{
	return uvm_atopg((vaddr_t)va);
}
#else
static inline struct page *
vmalloc_to_page(const void *va)
{
	vm_paddr_t paddr;

	paddr = pmap_kextract((vm_offset_t)va);
	return (struct page *)(PHYS_TO_VM_PAGE(paddr));
}
#endif

#if defined(__OpenBSD__)
static inline struct vm_page *
virt_to_page(const void *va)
{
	return uvm_atopg((vaddr_t)va);
}
#else /* which one to use from asm/page.h? */
#if 0
static inline struct page *
virt_to_page(void *kaddr)
{
	return (struct page *)PHYS_TO_VM_PAGE(vtophys(kaddr));
}
#endif
#define virt_to_page(kaddr)	(struct page *)PHYS_TO_VM_PAGE(vtophys(kaddr))
#endif

#if defined(__OpenBSD__)
static inline void *
kvcalloc(size_t n, size_t size, int flags)
{
	return kvmalloc_array(n, size, flags | M_ZERO);
}
#else
void *kvcalloc(size_t n, size_t size, int flags);
#endif

#if defined(__OpenBSD__)
static inline void *
kvzalloc(size_t size, int flags)
{
	return malloc(size, M_DRM, flags | M_ZERO);
}
#else
void *kvzalloc(size_t size, int flags);
#endif

#if defined(__OpenBSD__)
static inline void
kvfree(const void *objp)
{
	free((void *)objp, M_DRM, 0);
}
#else
#if 0 /* previous DragonFly */
#define kvfree(addr)	kfree(addr)
#endif
void kvfree(void *objp);
#endif

static inline unsigned int
get_order(size_t size)
{
#if defined(__OpenBSD__)
	return flsl((size - 1) >> PAGE_SHIFT);
#else
/*
 * Compute log2 of the power of two rounded up count of pages
 * needed for size bytes.
 */
	unsigned int order;

	size = (size - 1) >> PAGE_SHIFT;
	order = 0;
	while (size) {
		order++;
		size >>= 1;
	}
	return (order);
#endif
}

#if defined(__OpenBSD__)
static inline int
#else
static inline long
#endif
totalram_pages(void)
{
#if defined(__OpenBSD__)
	return uvmexp.npages;
#else
	return physmem;
#endif
}

#if 0
#define VM_FAULT_RETRY		0x0400
#endif

#define FAULT_FLAG_ALLOW_RETRY		0x04
#define FAULT_FLAG_RETRY_NOWAIT		0x08

struct vm_fault {
	struct vm_area_struct *vma;
	unsigned int flags;
	void __user *virtual_address;
};

#if 0
#define VM_FAULT_NOPAGE		0x0001
#define VM_FAULT_SIGBUS		0x0002
#define VM_FAULT_OOM		0x0004
#endif

#define VM_DONTDUMP	0x0001
#define VM_DONTEXPAND	0x0002
#define VM_IO		0x0004
#define VM_MIXEDMAP	0x0008

struct vm_operations_struct {
	int (*fault)(struct vm_area_struct *vma, struct vm_fault *vmf);
	void (*open)(struct vm_area_struct *vma);
	void (*close)(struct vm_area_struct *vma);
	int (*access)(struct vm_area_struct *vma, unsigned long addr,
		      void *buf, int len, int write);
};

/*
 * This only works via mmap ops.
 */
static inline int
io_remap_pfn_range(struct vm_area_struct *vma,
    unsigned long addr, unsigned long pfn, unsigned long size,
    vm_memattr_t prot)
{
	vma->vm_page_prot = prot;
	vma->vm_pfn = pfn;

	return (0);
}

static inline unsigned long
vma_pages(struct vm_area_struct *vma)
{
	unsigned long size;

	size = vma->vm_end - vma->vm_start;

	return size >> PAGE_SHIFT;
}



static inline void
get_page(struct vm_page *page)
{
	vm_page_hold(page);
}

extern vm_paddr_t Realmem;

static inline unsigned long get_num_physpages(void)
{
	return Realmem / PAGE_SIZE;
}


static inline void
unmap_mapping_range(struct address_space *mapping,
	loff_t const holebegin, loff_t const holelen, int even_cows)
{
}

#define VM_SHARED	0x00000008

#define VM_PFNMAP	0x00000400


static inline void
put_page(struct page *page)
{
	vm_page_busy_wait((struct vm_page *)page, FALSE, "i915gem");
	vm_page_unwire((struct vm_page *)page, 1);
	vm_page_wakeup((struct vm_page *)page);
}

static inline void *
page_address(const struct page *page)
{
	return (void *)VM_PAGE_TO_PHYS((const struct vm_page *)page);
}

#define FOLL_WRITE	0x01

struct sysinfo {
	uint64_t totalram;
	uint64_t totalhigh;
	uint32_t mem_unit;
};

void si_meminfo(struct sysinfo *si);

long get_user_pages(unsigned long start, unsigned long nr_pages,
		    unsigned int gup_flags, struct page **pages,
		    struct vm_area_struct **vmas);

void release_pages(struct page **pages, unsigned long nr_pages);

#endif	/* _LINUX_MM_H_ */

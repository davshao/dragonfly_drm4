/* Public domain. */	

/*
 * Copyright (c) 2015-2019 François Tigeot <ftigeot@wolfpond.org>
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

/* logic for clflush_cache_range from OpenBSD drm_cache.c */

/**************************************************************************
 *
 * Copyright (c) 2006-2007 Tungsten Graphics, Inc., Cedar Park, TX., USA
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
/*
 * Authors: Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 */

#ifndef _ASM_CACHEFLUSH_H
#define _ASM_CACHEFLUSH_H

#if defined(__OpenBSD__)
#include <machine/pmap.h>
#else
#include <machine/cpufunc.h>
#endif
// #include <vm/pmap.h>
// #include <vm/vm_page.h>

#include <asm/special_insns.h>

#if defined(__OpenBSD__)
#define clflush_cache_range(va, len)	pmap_flush_cache((vaddr_t)(va), len)
#else
static inline void
clflush_cache_range(void* va, unsigned int len)
{
	const int size = cpu_clflush_line_size;
	void* end = va + len;
	va = (void *)(((unsigned long)va) & ~size);
	cpu_mfence();
	for (; va < end; va+= size) {
		clflush((unsigned long)va);
	}
	clflush((unsigned long)(end - 1)); /* force serialisation */
	cpu_mfence();
}
#endif

#if 0
static inline int
set_memory_uc(unsigned long addr, int numpages)
{
	pmap_change_attr(addr, numpages, PAT_UNCACHED);
	return 0;
}

static inline int set_memory_wc(unsigned long vaddr, int numpages)
{
	pmap_change_attr(vaddr, numpages, PAT_WRITE_COMBINING);
	return 0;
}

static inline int set_memory_wb(unsigned long vaddr, int numpages)
{
	pmap_change_attr(vaddr, numpages, PAT_WRITE_BACK);
	return 0;
}
#endif

#if 0
static inline int
set_pages_uc(struct page *page, int num_pages)
{
	struct vm_page *p = (struct vm_page *)page;

	pmap_change_attr(PHYS_TO_DMAP(VM_PAGE_TO_PHYS(p)),
			 num_pages, PAT_UNCACHED);

	return 0;
}
#endif

#if 0
static inline int
set_pages_wb(struct page *page, int num_pages)
{
	struct vm_page *p = (struct vm_page *)page;

	pmap_change_attr(PHYS_TO_DMAP(VM_PAGE_TO_PHYS(p)),
			 num_pages, PAT_WRITE_BACK);

	return 0;
}
#endif

#if 0
static inline int
set_pages_array_uc(struct page **pages, int addrinarray)
{
	for (int i = 0; i < addrinarray; i++)
		pmap_page_set_memattr((struct vm_page *)pages[i], VM_MEMATTR_UNCACHEABLE);

	return 0;
}
#endif

#if 0
static inline int
set_pages_array_wb(struct page **pages, int addrinarray)
{
	for (int i = 0; i < addrinarray; i++)
		pmap_page_set_memattr((struct vm_page *)pages[i], VM_MEMATTR_WRITE_BACK);

	return 0;
}
#endif

#if 0
static inline int
set_pages_array_wc(struct page **pages, int addrinarray)
{
	for (int i = 0; i < addrinarray; i++)
		pmap_page_set_memattr((struct vm_page *)pages[i], VM_MEMATTR_WRITE_COMBINING);

	return 0;
}
#endif

#endif

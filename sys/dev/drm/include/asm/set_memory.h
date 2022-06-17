/*	$OpenBSD: set_memory.h,v 1.4 2022/01/14 06:53:14 jsg Exp $	*/
/*
 * Copyright (c) 2013, 2014, 2015 Mark Kettenis
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copyright (c) 2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _ASM_SET_MEMORY_H
#define _ASM_SET_MEMORY_H

#if defined(__OpenBSD__)
#include <sys/systm.h>
#include <sys/atomic.h>

#include <sys/param.h>		/* for PAGE_SIZE on i386 */
#include <uvm/uvm_extern.h>

#include <machine/pmap.h>
#else
#include <machine/cpufunc.h>
#include <vm/pmap.h>
#include <vm/vm_page.h>

#include <linux/atomic.h>
#endif

static inline int
#if defined(__OpenBSD__)
set_pages_array_wb(struct vm_page **pages, int addrinarray)
#else
set_pages_array_wb(struct page **pages, int addrinarray)
#endif
{
	int i;

	for (i = 0; i < addrinarray; i++)
#if defined(__OpenBSD__)
		atomic_clearbits_int(&pages[i]->pg_flags, PG_PMAP_WC);
#else
		pmap_page_set_memattr((struct vm_page *)pages[i], VM_MEMATTR_WRITE_BACK);
#endif

	return 0;
}

static inline int
#if defined(__OpenBSD__)
set_pages_array_wc(struct vm_page **pages, int addrinarray)
#else
set_pages_array_wc(struct page **pages, int addrinarray)
#endif
{
	int i;

	for (i = 0; i < addrinarray; i++)
#if defined(__OpenBSD__)
		atomic_setbits_int(&pages[i]->pg_flags, PG_PMAP_WC);
#else
		pmap_page_set_memattr((struct vm_page *)pages[i], VM_MEMATTR_WRITE_COMBINING);
#endif

	return 0;
}

static inline int
#if defined(__OpenBSD__)
set_pages_array_uc(struct vm_page **pages, int addrinarray)
#else
set_pages_array_uc(struct page **pages, int addrinarray)
#endif
{
#if defined(__OpenBSD__)
	/* XXX */
#else
	for (int i = 0; i < addrinarray; i++)
		pmap_page_set_memattr((struct vm_page *)pages[i], VM_MEMATTR_UNCACHEABLE);
#endif

	return 0;
}

static inline int
#if defined(__OpenBSD__)
set_pages_wb(struct vm_page *page, int numpages)
#else
set_pages_wb(struct page *page, int num_pages)
#endif
{
#if defined(__OpenBSD__)
	struct vm_page *pg;
	paddr_t start = VM_PAGE_TO_PHYS(page);
	int i;

	for (i = 0; i < numpages; i++) {
		pg = PHYS_TO_VM_PAGE(start + (i * PAGE_SIZE));
		if (pg != NULL)
			atomic_clearbits_int(&pg->pg_flags, PG_PMAP_WC);
	}
#else
	struct vm_page *p = (struct vm_page *)page;

	pmap_change_attr(PHYS_TO_DMAP(VM_PAGE_TO_PHYS(p)),
			 num_pages, PAT_WRITE_BACK);
#endif

	return 0;
}

static inline int
#if defined(__OpenBSD__)
set_pages_uc(struct vm_page *page, int numpages)
#else
set_pages_uc(struct page *page, int num_pages)
#endif
{
#if defined(__OpenBSD__)
	/* XXX */
#else
	struct vm_page *p = (struct vm_page *)page;

	pmap_change_attr(PHYS_TO_DMAP(VM_PAGE_TO_PHYS(p)),
			 num_pages, PAT_UNCACHED);
#endif
	return 0;
}

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

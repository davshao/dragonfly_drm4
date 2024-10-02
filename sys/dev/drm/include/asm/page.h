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

#ifndef _ASM_PAGE_H_
#define _ASM_PAGE_H_

#if 0
struct page;

#include <vm/vm_page.h>
#endif

#if 0
static inline vm_paddr_t
page_to_phys(struct page *page)
{
	struct vm_page *p = (struct vm_page *)page;

	return VM_PAGE_TO_PHYS(p);
}
#endif

// #define LINUX_PAGE_MASK	(~PAGE_MASK)

#if 0
static inline struct page *
virt_to_page(void *kaddr)
{
	return (struct page *)PHYS_TO_VM_PAGE(vtophys(kaddr));
}
#endif

// #include <asm/memory_model.h>

#if 0
#include <vm/vm_object.h>
#endif

#if 0
static inline unsigned long
page_to_pfn(struct page *page)
{
	struct vm_page *p = (struct vm_page *)page;

	return OFF_TO_IDX(VM_PAGE_TO_PHYS(p));
}
#endif

#if 0
static inline struct page *
pfn_to_page(unsigned long pfn)
{
	return (struct page *)PHYS_TO_VM_PAGE(pfn << PAGE_SHIFT);
}
#endif

// typedef unsigned long pgprot_t;

#if 0
#define virt_to_page(kaddr)	(struct page *)PHYS_TO_VM_PAGE(vtophys(kaddr))
#endif

#endif	/* _ASM_PAGE_H_ */

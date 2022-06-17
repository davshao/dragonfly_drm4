/* Public domain. */

/*
 * Copyright (c) 2015-2018 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _ASM_PGTABLE_H
#define _ASM_PGTABLE_H

#if defined(__OpenBSD__)
#include <machine/pmap.h>
#include <machine/pte.h>
#else
#include <sys/types.h>
#include <cpu/pmap.h>
#include <vm/vm.h>
#endif

#include <linux/types.h>

// #include <asm/page.h>

// #include <asm/pgtable_types.h>

#define pgprot_val(v)	(v)

#define _PAGE_PRESENT	X86_PG_V
#define _PAGE_RW	X86_PG_RW
#define _PAGE_PWT	X86_PG_NC_PWT
#define _PAGE_PCD	X86_PG_NC_PCD
#define _PAGE_ACCESSED	X86_PG_A
#define _PAGE_DIRTY	X86_PG_M
#define _PAGE_PAT	X86_PG_PTE_PAT
#define _PAGE_GLOBAL	X86_PG_G
#define _PAGE_NX	X86_PG_NX

#define _PAGE_CACHE_WC		_PAGE_PWT
#define _PAGE_CACHE_UC_MINUS	_PAGE_PCD

#define _PAGE_CACHE_MASK	(_PAGE_PWT | _PAGE_PCD | _PAGE_PAT)

#define __PAGE_KERNEL_EXEC						\
	(_PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_GLOBAL)
#define __PAGE_KERNEL	(__PAGE_KERNEL_EXEC | _PAGE_NX)

/* nonmatching between OpenBSD and DragonFly */
#if defined(__OpenBSD__)
#define PAGE_KERNEL	0
#define PAGE_KERNEL_IO	0
#else
#define PAGE_KERNEL	__PAGE_KERNEL
#define PAGE_KERNEL_IO	__PAGE_KERNEL
#endif

static inline pgprot_t
pgprot_writecombine(pgprot_t prot)
{
#if defined(__OpenBSD__)
#if PMAP_WC != 0
	return prot | PMAP_WC;
#else
	return prot | PMAP_NOCACHE;
#endif
#else /* DragonFly */
	return (prot | VM_MEMATTR_WRITE_COMBINING);
#endif
}

static inline pgprot_t
pgprot_noncached(pgprot_t prot)
{
#if defined(__OpenBSD__)
#if PMAP_DEVICE != 0
	return prot | PMAP_DEVICE;
#else
	return prot | PMAP_NOCACHE;
#endif
#else /* DragonFly */
	return (prot | VM_MEMATTR_UNCACHEABLE);
}
#endif

#define __pgprot(value)	((pgprot_t) {(value)})

#endif

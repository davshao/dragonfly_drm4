/*	$OpenBSD: drm_linux.c,v 1.94 2022/09/16 01:48:07 jsg Exp $	*/
/*
 * Copyright (c) 2013 Jonathan Gray <jsg@openbsd.org>
 * Copyright (c) 2015, 2016 Mark Kettenis <kettenis@openbsd.org>
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
 * Copyright (c) 2014-2020 François Tigeot <ftigeot@wolfpond.org>
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

#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/vm_map.h>
#include <linux/highmem.h>

void *
#if defined(__OpenBSD__)
kmap(struct vm_page *pg)
#else
kmap(struct page *pg)
#endif
{
#if defined(__OpenBSD__) /* drm_linux.c */
	vaddr_t va;

#if defined (__HAVE_PMAP_DIRECT) /* true on amd64 */
	va = pmap_map_direct(pg);
#else
	va = (vaddr_t)km_alloc(PAGE_SIZE, &kv_physwait, &kp_none, &kd_waitok);
	pmap_kenter_pa(va, VM_PAGE_TO_PHYS(pg), PROT_READ | PROT_WRITE);
	pmap_update(pmap_kernel());
#endif
	return (void *)va;
#else /* DragonFly */
	return (void *)PHYS_TO_DMAP(VM_PAGE_TO_PHYS( (struct vm_page *)pg ));
#endif
}


#if defined(__OpenBSD__)
void
kunmap_va(void *addr)
{
	vaddr_t va = (vaddr_t)addr;

#if defined (__HAVE_PMAP_DIRECT)
	pmap_unmap_direct(va);
#else
	pmap_kremove(va, PAGE_SIZE);
	pmap_update(pmap_kernel());
	km_free((void *)va, PAGE_SIZE, &kv_physwait, &kp_none);
#endif
}
#else
void
kunmap(struct page *pg)
{
	/* Nothing to do on systems with a direct memory map */
}
#endif

/* No longer used in 5.15+ kernel drm */
#if defined(__OpenBSD__)
#define kmap_to_page(ptr)	(ptr)
#else
struct page *
kmap_to_page(void *addr)
{
	if (addr == NULL)
		return NULL;

	return (struct page *)PHYS_TO_VM_PAGE(vtophys(addr));
}
#endif

#if defined(__OpenBSD__)
/* drm_linux.c
 * need to implement? OpenBSD makes an actual kernel map */
vaddr_t kmap_atomic_va;
#else
vm_offset_t kmap_atomic_va;
#endif
int kmap_atomic_inuse;

#if defined(__OpenBSD__)
void *
kmap_atomic_prot(struct vm_page *pg, pgprot_t prot)
{
	KASSERT(!kmap_atomic_inuse);

	kmap_atomic_inuse = 1;
	pmap_kenter_pa(kmap_atomic_va, VM_PAGE_TO_PHYS(pg) | prot,
	    PROT_READ | PROT_WRITE);
	return (void *)kmap_atomic_va;
}
#else
void *
kmap_atomic_prot(struct page *pg, pgprot_t prot)
{
	return (void *)PHYS_TO_DMAP(VM_PAGE_TO_PHYS( (struct vm_page *)pg ));
}
#endif

void
kunmap_atomic(void *addr)
{
/* drm_linux.c 
 * need to implement? OpenBSD removes an actual kernel map */
#if defined(__OpenBSD__)
	KASSERT(kmap_atomic_inuse);
	
	pmap_kremove(kmap_atomic_va, PAGE_SIZE);
	kmap_atomic_inuse = 0;
#else
	/* Nothing to do on systems with a direct memory map */
#endif
}

void *
#if defined(__OpenBSD__)
kmap_atomic(struct vm_page *pg)
#else
kmap_atomic(struct page *pg)
#endif
{
#if defined(__OpenBSD__)
	return kmap_atomic_prot(pg, PAGE_KERNEL);
#else
/* PAGE KERNEL is 0 on OpenBSD drm */
	return kmap_atomic_prot(pg, 0);
#endif
}

static int drm_page_init(void *dummy __unused)
{
	kmap_atomic_va = kmem_alloc_pageable(kernel_map, PAGE_SIZE, VM_SUBSYS_DRM);
	kmap_atomic_inuse = 0;
	return 0;
}

SYSINIT(linux_drm_page_init, SI_SUB_DRIVERS, SI_ORDER_MIDDLE, drm_page_init, NULL);

/*
 * Copyright (c) 2017-2019 François Tigeot <ftigeot@wolfpond.org>
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

#include <sys/queue.h>
#include <vm/vm_extern.h>

#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/mm.h>

struct vmap {
	void *addr;
	int npages;
	SLIST_ENTRY(vmap) vm_vmaps;
};

struct lock vmap_lock = LOCK_INITIALIZER("dlvml", 0, LK_CANRECURSE);

SLIST_HEAD(vmap_list_head, vmap) vmap_list = SLIST_HEAD_INITIALIZER(vmap_list);

/* vmap: map an array of pages into virtually contiguous space */
void *
vmap(struct page **pages, unsigned int npages, unsigned long flags,
    pgprot_t prot)
{
#if defined(__OpenBSD__)
	vaddr_t va;
	paddr_t pa;
	int i;

	va = (vaddr_t)km_alloc(PAGE_SIZE * npages, &kv_any, &kp_none,
	    &kd_nowait);
	if (va == 0)
		return NULL;
	for (i = 0; i < npages; i++) {
		pa = VM_PAGE_TO_PHYS(pages[i]) | prot;
		pmap_enter(pmap_kernel(), va + (i * PAGE_SIZE), pa,
		    PROT_READ | PROT_WRITE,
		    PROT_READ | PROT_WRITE | PMAP_WIRED);
		pmap_update(pmap_kernel());
	}

	return (void *)va;
#else
	struct vmap *vmp;
	vm_offset_t va;

	vmp = kmalloc(sizeof(struct vmap), M_DRM, M_WAITOK | M_ZERO);

	va = kmem_alloc_nofault(kernel_map, PAGE_SIZE * npages,
				 VM_SUBSYS_DRM_VMAP, PAGE_SIZE);
	if (va == 0)
		return (NULL);

	vmp->addr = (void *)va;
	vmp->npages = npages;
	pmap_qenter(va, (struct vm_page **)pages, npages);
	lockmgr(&vmap_lock, LK_EXCLUSIVE);
	SLIST_INSERT_HEAD(&vmap_list, vmp, vm_vmaps);
	lockmgr(&vmap_lock, LK_RELEASE);

	return (void *)va;
#endif
}

#if defined(__OpenBSD__)
void
vunmap(void *addr, size_t size)
#else
void
vunmap(const void *addr)
#endif
{
#if defined(__OpenBSD__)
	vaddr_t va = (vaddr_t)addr;

	pmap_remove(pmap_kernel(), va, va + size);
	pmap_update(pmap_kernel());
	km_free((void *)va, size, &kv_any, &kp_none);
#else
	struct vmap *vmp, *tmp_vmp;
	size_t size;

	SLIST_FOREACH_MUTABLE(vmp, &vmap_list, vm_vmaps, tmp_vmp) {
		if (vmp->addr == addr) {
			size = vmp->npages * PAGE_SIZE;

			pmap_qremove((vm_offset_t)addr, vmp->npages);
			kmem_free(kernel_map, (vm_offset_t)addr, size);
			goto found;
		}
	}

found:
	lockmgr(&vmap_lock, LK_EXCLUSIVE);
	SLIST_REMOVE(&vmap_list, vmp, vmap, vm_vmaps);
	lockmgr(&vmap_lock, LK_RELEASE);
	kfree(vmp);
#endif
}

/* not sure
 * might need 3 values
 * 0   not mapped in kernel_map
 * 1   mapped in kernel map
 * 2   mapped by vmap
 */
int
is_vmalloc_addr(const void *x)
{
#if defined(__OpenBSD__)
	vaddr_t min, max, addr;

	min = vm_map_min(kernel_map);
	max = vm_map_max(kernel_map);
	addr = (vaddr_t)p;

	if (addr >= min && addr <= max)
		return true;
	else
		return false;
#else
	struct vmap *vmp, *tmp_vmp;

	SLIST_FOREACH_MUTABLE(vmp, &vmap_list, vm_vmaps, tmp_vmp) {
		if (vmp->addr == x)
			return 1;
	}

	return false;
#endif
}

void *
vmalloc(unsigned long size)
{
	return __kmalloc(size, M_DRM, M_WAITOK);
}

void *
vzalloc(unsigned long size)
{
#if defined(__OpenBSD__)
	return malloc(size, M_DRM, M_WAITOK | M_CANFAIL | M_ZERO);
#else
	return __kmalloc(size, M_DRM, M_WAITOK | M_ZERO);
#endif
}

/* allocate zeroed virtually contiguous memory for userspace */
void *
vmalloc_user(unsigned long size)
{
	return __kmalloc(size, M_DRM, M_WAITOK | M_ZERO);
}

void
vfree(const void *addr)
{
	void *nc_addr;

	memcpy(&nc_addr, &addr, sizeof(void *));
	kfree(nc_addr);
}

void *
kvmalloc(size_t size, gfp_t flags)
{
	return __kmalloc(size, M_DRM, flags);
}

void *
kvmalloc_array(size_t n, size_t size, int flags)
{
#if defined(__OpenBSD__)
	if (n != 0 && SIZE_MAX / n < size)
		return NULL;
#else
	if (n == 0)
		return NULL;

	if (n > SIZE_MAX / size)
		return NULL;
#endif

	return __kmalloc(n * size, M_DRM, flags);
}

void *
kvcalloc(size_t n, size_t size, int flags)
{
	return kvmalloc_array(n, size, flags | M_ZERO);
}

void *
kvzalloc(size_t size, int flags)
{
	return __kmalloc(size, M_DRM, flags | M_ZERO);
}

void
kvfree(void *objp)
{
	kfree(objp);
}

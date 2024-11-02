/*
 * Copyright (c) 2014-2019 François Tigeot <ftigeot@wolfpond.org>
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

#include <machine/pmap.h>
#include <vm/pmap.h>
#include <vm/vm.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/io-mapping.h>
#include <asm/pgtable.h>
// #include <asm/page.h>

struct lock iomap_lock = LOCK_INITIALIZER("dlioml", 0, LK_CANRECURSE);

SLIST_HEAD(iomap_list_head, iomap) iomap_list = SLIST_HEAD_INITIALIZER(iomap_list);

void __iomem *
__ioremap_common(unsigned long phys_addr, unsigned long size, int cache_mode)
{
	struct iomap *imp;

	/* Ensure mappings are page-aligned */
	BUG_ON(phys_addr & PAGE_MASK);
	BUG_ON(size & PAGE_MASK);

	imp = __kmalloc(sizeof(struct iomap), M_DRM, M_WAITOK);
	imp->paddr = phys_addr;
	imp->npages = size / PAGE_SIZE;
	imp->pmap_addr = pmap_mapdev_attr(phys_addr, size, cache_mode);
	lockmgr(&iomap_lock, LK_EXCLUSIVE);
	SLIST_INSERT_HEAD(&iomap_list, imp, im_iomaps);
	lockmgr(&iomap_lock, LK_RELEASE);

	return imp->pmap_addr;
}

void iounmap(void __iomem *ptr)
{
	struct iomap *imp, *tmp_imp;
	int found = 0;
	int indx;
	vm_paddr_t paddr_end;

	SLIST_FOREACH_MUTABLE(imp, &iomap_list, im_iomaps, tmp_imp) {
		if (imp->pmap_addr == ptr) {
			found = 1;
			break;
		}
	}

	if (!found) {
		kprintf("iounmap: invalid address %p\n", ptr);
		return;
	}

	paddr_end = imp->paddr + (imp->npages * PAGE_SIZE) - 1;
	/* Is this address space range backed by regular memory ? */
	for (indx = 0; phys_avail[indx].phys_end != 0; ++indx) {
		vm_paddr_t range_start = phys_avail[indx].phys_beg;
		vm_paddr_t size = phys_avail[indx].phys_end -
				  phys_avail[indx].phys_beg;
		vm_paddr_t range_end = range_start + size - 1;

		if ((imp->paddr >= range_start) && (paddr_end <= range_end)) {
			/* Yes, change page caching attributes */
			pmap_change_attr(imp->paddr, imp->npages, PAT_WRITE_BACK);
			break;
		}

	}

	pmap_unmapdev((vm_offset_t)imp->pmap_addr, imp->npages * PAGE_SIZE);

	lockmgr(&iomap_lock, LK_EXCLUSIVE);
	SLIST_REMOVE(&iomap_list, imp, iomap, im_iomaps);
	lockmgr(&iomap_lock, LK_RELEASE);

	kfree(imp);
}

struct io_mapping *
io_mapping_create_wc(resource_size_t base, unsigned long size)
{
	struct io_mapping *map;

	map = __kmalloc(sizeof(struct io_mapping), M_DRM, M_WAITOK);
	map->base = base;
	map->size = size;
	map->prot = VM_MEMATTR_WRITE_COMBINING;

	map->vaddr = pmap_mapdev_attr(base, size,
					VM_MEMATTR_WRITE_COMBINING);
	if (map->vaddr == NULL)
		return NULL;

	return map;
}

void
io_mapping_free(struct io_mapping *mapping)
{
	/* Default memory attribute is write-back */
	pmap_mapdev_attr(mapping->base, mapping->size, VM_MEMATTR_WRITE_BACK);
	kfree(mapping);
}

void *
io_mapping_map_wc(struct io_mapping *mapping,
		  unsigned long offset,
		  unsigned long size)
{
	BUG_ON(offset >= mapping->size);

	return ioremap_wc(mapping->base + offset, size);
}

void *
io_mapping_map_atomic_wc(struct io_mapping *mapping, unsigned long offset)
{
	return ioremap_wc(mapping->base + offset, PAGE_SIZE);
}

void
io_mapping_unmap(void *vaddr)
{
	iounmap(vaddr);
}

void
io_mapping_unmap_atomic(void *vaddr)
{
	iounmap(vaddr);
}

struct io_mapping *
io_mapping_init_wc(struct io_mapping *iomap,
		   resource_size_t base,
		   unsigned long size)
{
	iomap->base = base;
	iomap->size = size;
	iomap->vaddr = ioremap_wc(base, size);
	iomap->prot = pgprot_writecombine(PAGE_KERNEL_IO);

	return iomap;
}

void
io_mapping_fini(struct io_mapping *mapping)
{
	iounmap(mapping->vaddr);
}

#include <sys/memrange.h>

int
arch_io_reserve_memtype_wc(resource_size_t start, resource_size_t size)
{
	int act;
	struct mem_range_desc mrdesc;

	mrdesc.mr_base = start;
	mrdesc.mr_len = size;
	mrdesc.mr_flags = MDF_WRITECOMBINE;
	act = MEMRANGE_SET_UPDATE;
	strlcpy(mrdesc.mr_owner, "drm", sizeof(mrdesc.mr_owner));
	return mem_range_attr_set(&mrdesc, &act);
}

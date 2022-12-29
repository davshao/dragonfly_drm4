/*	$OpenBSD: highmem.h,v 1.4 2021/07/28 13:28:05 kettenis Exp $	*/
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

#ifndef _LINUX_HIGHMEM_H
#define _LINUX_HIGHMEM_H

#if defined(__OpenBSD__)
#include <sys/param.h>
#include <uvm/uvm_extern.h>
#else
#include <machine/vmparam.h>
#endif

#include <linux/uaccess.h>
#include <asm/pgtable.h>

#include <asm/cacheflush.h> /* set_memory_wb() commented out OpenBSD */

#if 0
#include <linux/kernel.h>
#include <linux/bug.h>
#include <linux/mm.h>
#include <linux/hardirq.h>
#endif

#if defined(__OpenBSD__)
/* drm_linux.c
 * need to implement? OpenBSD makes an actual kernel map */
vaddr_t kmap_atomic_va;
#else
extern vm_offset_t kmap_atomic_va;
#endif
extern int kmap_atomic_inuse;

#if defined(__OpenBSD__)
void	*kmap(struct vm_page *);
#else
void	*kmap(struct page *);
#endif

#if defined(__OpenBSD__)
void	kunmap_va(void *addr);
#else
void	kunmap(struct page *pg);
#endif

/* No longer used in 5.15+ kernel drm */
#if defined(__OpenBSD__)
#define kmap_to_page(ptr)	(ptr)
#else
struct page *kmap_to_page(void *addr);
#endif

#if defined(__OpenBSD__)
void	*kmap_atomic_prot(struct vm_page *, pgprot_t);
#else
void	*kmap_atomic_prot(struct page *, pgprot_t);
#endif

void	kunmap_atomic(void *);

void *
#if defined(__OpenBSD__)
kmap_atomic(struct vm_page *pg);
#else
kmap_atomic(struct page *pg);
#endif

#endif

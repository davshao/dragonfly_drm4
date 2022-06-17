/*	$OpenBSD: scatterlist.h,v 1.4 2021/07/07 02:38:36 jsg Exp $	*/
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
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013, 2014 Mellanox Technologies, Ltd.
 * Copyright (c) 2015 Matthew Dillon <dillon@backplane.com>
 * Copyright (c) 2016 Matt Macy <mmacy@nextbsd.org>
 * Copyright (c) 2017-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_SCATTERLIST_H
#define _LINUX_SCATTERLIST_H

#include <linux/string.h>
#include <linux/types.h>
#include <linux/bug.h>
#include <linux/mm.h>
// #include <asm/io.h>

/*
 * SG table design.
 *
 * If flags bit 0 is set, then the sg field contains a pointer to the next sg
 * table list. Otherwise the next entry is at sg + 1, can be determined using
 * the sg_is_chain() function.
 *
 * If flags bit 1 is set, then this sg entry is the last element in a list,
 * can be determined using the sg_is_last() function.
 *
 * See sg_next().
 *
 */

struct scatterlist {
#if defined(__OpenBSD__)
	struct vm_page *__page;
#else
	union {
		struct page		*page;
		struct scatterlist	*sg;
	} sl_un;
#endif
	dma_addr_t dma_address;
#if defined(__OpenBSD__)
	unsigned int offset;
#else
	unsigned long offset;
#endif
	unsigned int length;
	uint32_t flags;
	bool end;
};

struct sg_table {
	struct scatterlist *sgl;
	unsigned int nents;
	unsigned int orig_nents;
};

struct sg_page_iter {
	struct scatterlist *sg;
	unsigned int sg_pgoffset;
	unsigned int __nents;
	unsigned int maxents;
	int __pg_advance;
};

#define	SG_END		0x01
#define	SG_CHAIN	0x02

#if defined(__OpenBSD__)
#define sg_is_chain(sg)		false
#else
static inline bool
sg_is_chain(struct scatterlist *sg)
{
	return (sg->flags & SG_CHAIN);
}
#endif

#if defined(__OpenBSD__)
#define sg_is_last(sg)		((sg)->end)
#else
static inline bool
sg_is_last(struct scatterlist *sg)
{
	return (sg->flags & SG_END);
}
#endif

#if defined(__OpenBSD__)
#define sg_chain_ptr(sg)	NULL
#else
static inline struct scatterlist *
sg_chain_ptr(struct scatterlist *sg)
{
	return sg->sl_un.sg;
}
#endif

#define	sg_scatternext(sgl)	(sgl)->sl_un.sg

static inline struct scatterlist *
sg_next(struct scatterlist *sgl)
{
	if (sgl->flags & SG_END)
		return (NULL);
	sgl++;
	if (sgl->flags & SG_CHAIN)
		sgl = sg_scatternext(sgl);
	return (sgl);
}

#define SCATTERLIST_MAX_SEGMENT	(UINT_MAX & LINUX_PAGE_MASK)

/*
 * Maximum number of entries that will be allocated in one piece, if
 * a list larger than this is required then chaining will be utilized.
 */
#define SG_MAX_SINGLE_ALLOC             (PAGE_SIZE / sizeof(struct scatterlist))

int sg_alloc_table(struct sg_table *, unsigned int, gfp_t);
void sg_free_table(struct sg_table *);


static inline void
sg_mark_end(struct scatterlist *sgl)
{
	sgl->end = true;
        sgl->flags = SG_END;
}

static inline void
__sg_page_iter_start(struct sg_page_iter *iter, struct scatterlist *sgl,
    unsigned int nents, unsigned long pgoffset)
{
	iter->sg = sgl;
	iter->sg_pgoffset = pgoffset;
	iter->__nents = nents;
	iter->__pg_advance = 0;
}


static inline int
sg_page_count(struct scatterlist *sg)
{
	return PAGE_ALIGN(sg->offset + sg->length) >> PAGE_SHIFT;
}

static inline bool
__sg_page_iter_next(struct sg_page_iter *iter)
{
	if (iter->__nents == 0)
		return (false);
	if (iter->sg == NULL)
		return (false);

	iter->sg_pgoffset += iter->__pg_advance;
	iter->__pg_advance = 1;

	while (iter->sg_pgoffset >= sg_page_count(iter->sg)) {
		iter->sg_pgoffset -= sg_page_count(iter->sg);
		iter->sg = sg_next(iter->sg);
		if (--iter->__nents == 0)
			return (false);
		if (iter->sg == NULL)
			return (false);
	}
	return (true);
}

static inline dma_addr_t
sg_page_iter_dma_address(struct sg_page_iter *spi)
{
	return spi->sg->dma_address + (spi->sg_pgoffset << PAGE_SHIFT);
}

#if defined(__OpenBSD__)
static inline struct vm_page *
sg_page(struct scatterlist *sgl)
{
	return sgl->__page;
}
#else
#define	sg_page(sgl)		(sgl)->sl_un.page
#endif

static inline struct page *
sg_page_iter_page(struct sg_page_iter *piter)
{
	return nth_page(sg_page(piter->sg), piter->sg_pgoffset);
}

static inline void
sg_assign_page(struct scatterlist *sgl, struct page *page)
{
#if defined(__OpenBSD__)
	sgl->__page = page;
#else
	sgl->sl_un.page = page;
#endif
}

#define	sg_dma_len(sg)		(sg)->length

static inline void
sg_set_page(struct scatterlist *sgl, struct page *page,
    unsigned int length, unsigned int offset)
{
#if defined(__OpenBSD__)
	sgl->__page = page;
#else
	sg_page(sgl) = page;
#endif
#if defined(__OpenBSD__)
	sgl->dma_address = page ? VM_PAGE_TO_PHYS(page) : 0;
#else /* not sure */
	sgl->dma_address = page ? VM_PAGE_TO_PHYS(&page->pa_vmpage) : 0;
#endif
	sgl->offset = offset;
	sg_dma_len(sgl) = length;
	if (offset > PAGE_SIZE)
		panic("sg_set_page: Invalid offset %d\n", offset);
/* not sure */
	sgl->end = false;
}

/*
 * Iterate pages in sg list.
 */
static inline void
__sg_iter_next(struct sg_page_iter *iter)
{
	struct scatterlist *sg;
	unsigned int pgcount;

	sg = iter->sg;
	pgcount = (sg->offset + sg->length + PAGE_MASK) >> PAGE_SHIFT;

	++iter->sg_pgoffset;
	while (iter->sg_pgoffset >= pgcount) {
		iter->sg_pgoffset -= pgcount;
		sg = sg_next(sg);
		--iter->maxents;
		if (sg == NULL || iter->maxents == 0)
			break;
		pgcount = (sg->offset + sg->length + PAGE_MASK) >> PAGE_SHIFT;
	}
	iter->sg = sg;
}

/*
 * NOTE: pgoffset is really a page index, not a byte offset.
 */
static inline void
_sg_iter_init(struct scatterlist *sgl, struct sg_page_iter *iter,
	      unsigned int nents, unsigned long pgoffset)
{
	if (nents) {
		/*
		 * Nominal case.  Note subtract 1 from starting page index
		 * for initial __sg_iter_next() call.
		 */
		iter->sg = sgl;
		iter->sg_pgoffset = pgoffset - 1;
		iter->maxents = nents;
		__sg_iter_next(iter);
	} else {
		/*
		 * Degenerate case
		 */
		iter->sg = NULL;
		iter->sg_pgoffset = 0;
		iter->maxents = 0;
	}
}

#define	sg_dma_address(sg)	(sg)->dma_address

#define	for_each_sg(sgl, sg, nents, i)				\
	for (i = 0, sg = (sgl); i < (nents); i++, sg = sg_next(sg))

#define for_each_sg_page(sgl, iter, nents, pgoffset)			\
	for (_sg_iter_init(sgl, iter, nents, pgoffset);			\
	     (iter)->sg; __sg_iter_next(iter))

static inline void
sg_init_table(struct scatterlist *sg, unsigned int nents)
{
	bzero(sg, sizeof(*sg) * nents);
	sg[nents - 1].flags = SG_END;
}


static inline vm_paddr_t
sg_phys(struct scatterlist *sg)
{
	return ((struct vm_page *)sg_page(sg))->phys_addr + sg->offset;
}

static inline int
sg_nents(struct scatterlist *sg)
{
	int nents;
	for (nents = 0; sg; sg = sg_next(sg))
		nents++;
	return nents;
}

/*
 *
 * XXX please review these
 */
size_t sg_pcopy_from_buffer(struct scatterlist *sgl, unsigned int nents,
		      const void *buf, size_t buflen, off_t skip);

static inline size_t
sg_copy_from_buffer(struct scatterlist *sgl, unsigned int nents,
		     const char *buf, size_t buflen)
{
	return (sg_pcopy_from_buffer(sgl, nents, buf, buflen, 0));
}

size_t sg_pcopy_to_buffer(struct scatterlist *sgl, unsigned int nents,
		   void *buf, size_t buflen, off_t skip);

static inline size_t
sg_copy_to_buffer(struct scatterlist *sgl, unsigned int nents,
		  char *buf, size_t buflen)
{

	return (sg_pcopy_to_buffer(sgl, nents, buf, buflen, 0));
}

static inline int
sg_alloc_table_from_pages(struct sg_table *sgt,
	struct page **pages, unsigned int n_pages,
	unsigned long offset, unsigned long size, gfp_t gfp_mask)
{
	kprintf("sg_alloc_table_from_pages: Not implemented\n");
	return -EINVAL;
}

int __sg_alloc_table_from_pages(struct sg_table *sgt, struct page **pages,
				unsigned int n_pages, unsigned int offset,
				unsigned long size, unsigned int max_segment,
				gfp_t gfp_mask);

#endif

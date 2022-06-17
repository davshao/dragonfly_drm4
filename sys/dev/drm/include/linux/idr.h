/*
 * Copyright (c) 2013-2016 François Tigeot
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

#ifndef _LINUX_IDR_H
#define _LINUX_IDR_H

#include <sys/types.h>
#if defined(__OpenBSD__)
#include <sys/tree.h>
#else
#include <sys/idr.h>
#endif

#if defined(__OpenBSD__)
#include <linux/radix-tree.h>
#else
#include <linux/slab.h>
#endif

#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_IDR);
#endif

#if defined(__OpenBSD__)
struct idr_entry {
	SPLAY_ENTRY(idr_entry) entry;
	unsigned long id;
	void *ptr;
};

struct idr {
	SPLAY_HEAD(idr_tree, idr_entry) tree;
};
#endif

#if defined(__OpenBSD__)
void idr_init(struct idr *);
void idr_preload(unsigned int);
int idr_alloc(struct idr *, void *, int, int, gfp_t);
void *idr_find(struct idr *, unsigned long);
#endif
#if defined(__OpenBSD__) /* last argument int on DragonFly */
void *idr_replace(struct idr *, void *, unsigned long);
void *idr_remove(struct idr *, unsigned long);
#endif
#if defined(__OpenBSD__)
void idr_destroy(struct idr *);
int idr_for_each(struct idr *, int (*)(int, void *, void *), void *);
#endif
#if defined(__OpenBSD__) /* not defined on DragonFly */
void *idr_get_next(struct idr *, int *);
#else
static inline void *
idr_get_next(struct idr *idr, int *nextid)
{
	void *res = NULL;

	lwkt_gettoken(&idr->idr_token);

	for (int id = *nextid; id < idr->idr_count; id++) {
		res = idr_find(idr, id);
		if (res == NULL)
			continue;
		*nextid = id;
		break;
	}

	lwkt_reltoken(&idr->idr_token);
	return res;
}
#endif

#if defined(__OpenBSD__)
#define idr_for_each_entry(idp, entry, id) \
	for (id = 0; ((entry) = idr_get_next(idp, &(id))) != NULL; id++)
#else
#define idr_for_each_entry(idp, entry, id)			\
	for (id = 0; ((entry) = idr_get_next(idp, &(id))) != NULL; ++id)
#endif
 
static inline void
idr_init_base(struct idr *idr, int base)
{
	idr_init(idr);
}

#if defined(__OpenBSD__)
static inline void
idr_preload_end(void)
{
}
#endif
 
static inline bool
idr_is_empty(const struct idr *idr)
{
#if defined(__OpenBSD__)
	return SPLAY_EMPTY(&idr->tree);
#else
/* Not sure */
	int i;
	struct idr_node *nodes;
	lwkt_gettoken(&idr->idr_token);

	nodes = idr->idr_nodes;
	for (int id = 0; id < idr->idr_count; id++) {
		if (nodes[i].allocated > 0) {
			lwkt_reltoken(&idr->idr_token);
			return false;
		}
	}

	lwkt_reltoken(&idr->idr_token);
	return true;
}
#endif


#define	IDA_CHUNK_SIZE		128	/* 128 bytes per chunk */
#define	IDA_BITMAP_LONGS	(IDA_CHUNK_SIZE / sizeof(long) - 1)
#define	IDA_BITMAP_BITS 	(IDA_BITMAP_LONGS * sizeof(long) * 8)

struct ida_bitmap {
	long			nr_busy;
	unsigned long		bitmap[IDA_BITMAP_LONGS];
};

struct ida {
	struct idr		idr;
	struct ida_bitmap	*free_bitmap;
};

#define DEFINE_IDA(name)		\
	struct ida name = {		\
		.idr = { .idr_token = LWKT_TOKEN_INITIALIZER("idrtok") }, \
		.free_bitmap = NULL	\
	}				\

static inline void
ida_init(struct ida *ida)
{
	idr_init(&ida->idr);
}

static inline void
ida_destroy(struct ida *ida)
{
	idr_destroy(&ida->idr);
	if (ida->free_bitmap != NULL) {
		/* kfree() is a linux macro! Work around the cpp pass */
		_kfree(ida->free_bitmap, M_IDR);
	}
}

static inline int
ida_simple_get(struct ida *ida, unsigned int start, unsigned int end, gfp_t gfp_mask)
{
	int id;
	unsigned int lim;

	if ((end == 0) || (end > 0x80000000))
		lim = 0x80000000;
	else
		lim = end - 1;

	idr_preload(gfp_mask);
	id = idr_alloc(&ida->idr, NULL, start, lim, gfp_mask);
	idr_preload_end();

	return id;
}

static inline void
ida_simple_remove(struct ida *ida, unsigned int id)
{
	idr_remove(&ida->idr, id);
}

static inline void
ida_remove(struct ida *ida, int id)
{
	idr_remove(&ida->idr, id);
}

#endif

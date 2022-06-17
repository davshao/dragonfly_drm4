/*	$OpenBSD: dma-buf.h,v 1.4 2022/03/01 04:08:04 jsg Exp $	*/
/*
 * Copyright (c) 2018 Mark Kettenis
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
 * Copyright (c) 2018-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_DMA_BUF_H
#define _LINUX_DMA_BUF_H

#include <sys/types.h>
#include <sys/systm.h>
#include <linux/dma-resv.h>
#include <linux/list.h>
// #include <linux/err.h>
// #include <linux/scatterlist.h>
// #include <linux/dma-mapping.h>
#include <linux/dma-direction.h>
// #include <linux/fs.h>
// #include <linux/dma-fence.h>
// #include <linux/wait.h>

// #include <linux/slab.h>

struct dma_buf_ops;
struct device;

struct dma_buf {
	const struct dma_buf_ops *ops;
	void *priv;
	size_t size;
	struct file *file;
	struct list_head attachments;
	struct dma_resv *resv;
};

struct dma_buf_attachment {
	void *importer_priv;
	struct dma_buf *dmabuf;
	struct device *dev;
	void *priv;
};

struct dma_buf_attach_ops {
	void (*move_notify)(struct dma_buf_attachment *);
	bool allow_peer2peer;
};

void	get_dma_buf(struct dma_buf *);

#if 0 /* previous DragonFly */
static inline void
get_dma_buf(struct dma_buf *dmabuf)
{
#if defined (__OpenBSD__) /* from dma_linux.c */
	FREF(dmabuf->file);
#else
	fhold(dmabuf->file);
#endif
}
#endif

struct dma_buf *dma_buf_get(int);

void	dma_buf_put(struct dma_buf *);

#if 0 /* previous DragonFly */
static inline void
dma_buf_put(struct dma_buf *dmabuf)
{
	if (dmabuf == NULL)
		return;

	if (dmabuf->file == NULL)
		return;

	fdrop(dmabuf->file);
}
#endif

int dma_buf_fd(struct dma_buf *, int);

struct sg_table;
struct vm_area_struct;

struct dma_buf_ops {
	struct sg_table * (*map_dma_buf)(struct dma_buf_attachment *,
						enum dma_data_direction);
	void (*unmap_dma_buf)(struct dma_buf_attachment *,
						struct sg_table *,
						enum dma_data_direction);
	void (*release)(struct dma_buf *);
	void *(*map)(struct dma_buf *, unsigned long);
	void *(*map_atomic)(struct dma_buf *, unsigned long);
	void (*unmap)(struct dma_buf *, unsigned long, void *);
	void (*unmap_atomic)(struct dma_buf *, unsigned long, void *);
	int (*mmap)(struct dma_buf *, struct vm_area_struct *vma);
	void *(*vmap)(struct dma_buf *);
	void (*vunmap)(struct dma_buf *, void *vaddr);
	int (*begin_cpu_access)(struct dma_buf *, enum dma_data_direction);
	int (*end_cpu_access)(struct dma_buf *, enum dma_data_direction);
	int (*attach)(struct dma_buf *, struct device *, struct dma_buf_attachment *);
	void (*detach)(struct dma_buf *, struct dma_buf_attachment *);
};

struct dma_buf_export_info {
	const struct dma_buf_ops *ops;
	size_t size;
	int flags;
	void *priv;
	struct dma_resv *resv;
};

#if defined(__OpenBSD__)
#define DEFINE_DMA_BUF_EXPORT_INFO(x)  struct dma_buf_export_info x
#else
#define DEFINE_DMA_BUF_EXPORT_INFO(x)  struct dma_buf_export_info x = {}
#endif

struct dma_buf *dma_buf_export(const struct dma_buf_export_info *);

struct dma_buf_attachment *dma_buf_attach(struct dma_buf *, struct device *);
#if 0 /* previous DragonFly */
static inline struct dma_buf_attachment *
dma_buf_attach(struct dma_buf *buf, struct device *dev)
{
	/* XXX: this function is a stub */
	struct dma_buf_attachment *attach;

	attach = kmalloc(sizeof(struct dma_buf_attachment), M_DRM, M_WAITOK | M_ZERO);

	return attach;
}
#endif

void dma_buf_detach(struct dma_buf *, struct dma_buf_attachment *);
#if 0 /* previous DragonFly */
static inline void
dma_buf_detach(struct dma_buf *buf, struct dma_buf_attachment *dba)
{
#if defined(__OpenBSD__)
	panic("dma_buf_detach");
#else
	kprintf("dma_buf_detach: Not implemented\n");
#endif
}
#endif

struct sg_table * dma_buf_map_attachment(struct dma_buf_attachment *,
						enum dma_data_direction);
void dma_buf_unmap_attachment(struct dma_buf_attachment *,
				struct sg_table *, enum dma_data_direction);

#endif

/*
 * Copyright (c) 2019-2020 François Tigeot <ftigeot@wolfpond.org>
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

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/dma-buf.h>
#include <linux/dma-fence.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/dma-resv.h>
#include <linux/mm.h>
#include <linux/dma-direction.h>
#include <linux/fs.h>

static int
dmabuf_stat(struct file *fp, struct stat *sb, struct ucred *cred)
{
	struct dma_buf *dmabuf = fp->f_data;

	memset(sb, 0, sizeof(*sb));
	sb->st_size = dmabuf->size;
	sb->st_mode = S_IFIFO;	/* XXX */

	return (0);
}

static int
dmabuf_close(struct file *fp)
{
	kprintf("dmabuf_close(): not implemented\n");
	return (EINVAL);
}

struct fileops dmabufops = {
	.fo_read	= badfo_readwrite,
	.fo_write	= badfo_readwrite,
	.fo_ioctl	= badfo_ioctl,
	.fo_kqfilter	= badfo_kqfilter,
	.fo_stat	= dmabuf_stat,
	.fo_close	= dmabuf_close,
};

struct dma_buf *
dma_buf_export(const struct dma_buf_export_info *info)
{
#if defined(__OpenBSD__)
	struct proc *p = curproc;
#endif
	struct dma_buf *dmabuf;
	struct file *fp;

#if defined(__OpenBSD__)
	fp = fnew(p);
#else
	falloc(curthread->td_lwp, &fp, NULL);
#endif
	if (fp == NULL)
		return ERR_PTR(-ENFILE);
	fp->f_type = DTYPE_DMABUF;
	fp->f_ops = &dmabufops;
#if defined(__OpenBSD__)
	dmabuf = malloc(sizeof(struct dma_buf), M_DRM, M_WAITOK | M_ZERO);
#else
	dmabuf = __kmalloc(sizeof(struct dma_buf), M_DRM, M_WAITOK | M_ZERO);
#endif
	dmabuf->priv = info->priv;
	dmabuf->ops = info->ops;
	dmabuf->size = info->size;
	dmabuf->file = fp;
#if defined(__OpenBSD__)
	fp->f_data = dmabuf;
#else
	fp->private_data = dmabuf;
#endif
	INIT_LIST_HEAD(&dmabuf->attachments);
	return dmabuf;
}

struct dma_buf *
dma_buf_get(int fd)
{
#if defined(__OpenBSD__) /* drm_linux.c */
	struct proc *p = curproc;
	struct filedesc *fdp = p->p_fd;
	struct file *fp;

	if ((fp = fd_getfile(fdp, fd)) == NULL)
		return ERR_PTR(-EBADF);

	if (fp->f_type != DTYPE_DMABUF) {
		FRELE(fp, p);
		return ERR_PTR(-EINVAL);
	}

	return fp->f_data;
#else
	struct file *fp;
	struct dma_buf *dmabuf;

	if ((fp = holdfp(curthread, fd, -1)) == NULL)
		return ERR_PTR(-EBADF);

	if (fp->f_ops != &dmabufops) {
		kprintf("dma_buf_get(): file->f_ops != &dmabufops\n");
		fdrop(fp);
		return ERR_PTR(-EBADF);
	}

	dmabuf = fp->private_data;
	fdrop(fp);

	return dmabuf;
#endif
}

void
dma_buf_put(struct dma_buf *dmabuf)
{
#if defined(__OpenBSD__) /* from drm_linux.c */
	KASSERT(dmabuf);
	KASSERT(dmabuf->file);

	FRELE(dmabuf->file, curproc);
#else
	if (dmabuf == NULL)
		return;

	if (dmabuf->file == NULL)
		return;

	fdrop(dmabuf->file);
#endif
}

int
dma_buf_fd(struct dma_buf *dmabuf, int flags)
{
#if defined(__OpenBSD__)
	struct proc *p = curproc;
	struct filedesc *fdp = p->p_fd;
	struct file *fp = dmabuf->file;
	int fd, cloexec, error;

	cloexec = (flags & O_CLOEXEC) ? UF_EXCLOSE : 0;

	fdplock(fdp);
restart:
	if ((error = fdalloc(p, 0, &fd)) != 0) {
		if (error == ENOSPC) {
			fdexpand(p);
			goto restart;
		}
		fdpunlock(fdp);
		return -error;
	}

	fdinsert(fdp, fd, cloexec, fp);
	fdpunlock(fdp);

	return fd;
#else
	int fd, error;

	if (dmabuf == NULL)
		return -EINVAL;

	if (dmabuf->file == NULL)
		return -EINVAL;

	if (flags & O_CLOEXEC) {
	/* XXX: CLOEXEC not handled yet */
#if 0
		__set_close_on_exec(fd, fdt);
	else
		__clear_close_on_exec(fd, fdt);
#endif
	}

	error = fdalloc(curproc, 0, &fd);
	if (error != 0)
		return -error;

	fsetfd(curproc->p_fd, dmabuf->file, fd);

	return fd;
#endif
}

void
get_dma_buf(struct dma_buf *dmabuf)
{
#if defined (__OpenBSD__) /* from dma_linux.c */
	FREF(dmabuf->file);
#else
	fhold(dmabuf->file);
#endif
}

struct dma_buf_attachment *
dma_buf_attach(struct dma_buf *buf, struct device *dev)
{
#if defined(__OpenBSD__)
	return NULL;
#else
	/* XXX: this function is a stub */
	struct dma_buf_attachment *attach;

	attach = __kmalloc(sizeof(struct dma_buf_attachment), M_DRM, M_WAITOK | M_ZERO);

	return attach;
#endif
}

void
dma_buf_detach(struct dma_buf *buf, struct dma_buf_attachment *dba)
{
#if defined(__OpenBSD__)
	panic("dma_buf_detach");
#else
	kprintf("dma_buf_detach: Not implemented\n");
#endif
}

struct sg_table *
dma_buf_map_attachment(struct dma_buf_attachment *attach,
				enum dma_data_direction direction)
{
	struct sg_table *sg_table;

	if (attach == NULL)
		return ERR_PTR(-EINVAL);

	if (attach->dmabuf == NULL)
		return ERR_PTR(-EINVAL);

	sg_table = attach->dmabuf->ops->map_dma_buf(attach, direction);
	if (sg_table == NULL)
		return ERR_PTR(-ENOMEM);

	return sg_table;
}

void dma_buf_unmap_attachment(struct dma_buf_attachment *attach,
				struct sg_table *sg_table,
				enum dma_data_direction direction)
{
}

/*-
 * Copyright (c) 2020 Mellanox Technologies, Ltd.
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
 *
 * $FreeBSD$
 */

/* Public domain. */

#ifndef _LINUX_XARRAY_H
#define _LINUX_XARRAY_H

/* Replace mutex with lock for DragonFly */

#include <sys/lock.h>

#include <linux/gfp.h>
#include <linux/radix-tree.h>
#include <linux/err.h>

#define	XA_LIMIT(min, max) \
    ({ CTASSERT((min) == 0); (uint32_t)(max); })

#define XA_FLAGS_ALLOC		1
#define XA_FLAGS_ALLOC1		2
#define XA_FLAGS_LOCK_IRQ	4

#define	XA_ERROR(x) \
	ERR_PTR(x)

#define	xa_limit_32b XA_LIMIT(0, 0xFFFFFFFF)

struct xarray {
	struct radix_tree_root root;
	struct lock mtx;	/* internal mutex */
	uint32_t flags;		/* see XA_FLAGS_XXX */
};

#if defined(__FreeBSD__)
#define	XA_ASSERT_LOCKED(xa) mtx_assert(&(xa)->mtx, MA_OWNED)
#define	xa_lock(xa) mtx_lock(&(xa)->mtx)
#define	xa_unlock(xa) mtx_unlock(&(xa)->mtx)
#else /* DragonFly */
#define	xa_lock(xa) lockmgr(&(xa)->mtx, LK_EXCLUSIVE)
#define	xa_unlock(xa) lockmgr(&(xa)->mtx, LK_RELEASE)
#endif

/*
 * OpenBSD lock functions found in kernel drm graphics
 */
#define xa_lock_irqsave(_xa, _flags) do {		\
		_flags = 0;				\
		lockmgr(&(_xa)->mtx, LK_EXCLUSIVE);	\
	} while (0)

#define xa_unlock_irqrestore(_xa, _flags) do {		\
		(void)(_flags);				\
		lockmgr(&(_xa)->mtx, LK_RELEASE);	\
	} while (0)

/*
 * FreeBSD API
 * Functions actually called in kernel drm graphics
 */
/*
 * Extensible arrays API implemented as a wrapper
 * around the radix tree implementation.
 */
void *xa_erase(struct xarray *, uint32_t);
void *xa_load(struct xarray *, uint32_t);
int xa_alloc(struct xarray *, uint32_t *, void *, uint32_t, gfp_t);
void *xa_store(struct xarray *, uint32_t, void *, gfp_t);
void xa_init_flags(struct xarray *, uint32_t);
bool xa_empty(struct xarray *);
void xa_destroy(struct xarray *);

#if defined(__OpenBSD__)
#define xa_for_each(xa, index, entry) \
	for (index = 0; ((entry) = xa_get_next(xa, &(index))) != NULL; index++)
#else /* FreeBSD and now DragonFly */
#define	xa_for_each(xa, index, entry) \
	for ((entry) = NULL, (index) = 0; \
	     ((entry) = xa_next(xa, &index, (entry) != NULL)) != NULL; )
#endif

/*
 * Unlocked version of functions above.
 */
void *__xa_erase(struct xarray *, uint32_t);
int __xa_alloc_cyclic(struct xarray *, uint32_t *, void *, uint32_t, uint32_t *, gfp_t);
void *__xa_store(struct xarray *, uint32_t, void *, gfp_t);

/*
 * Functions FreeBSD implements
 * never mentioned in kernel drm graphics
 */
int xa_alloc_cyclic(struct xarray *, uint32_t *, void *, uint32_t, uint32_t *, gfp_t);
int xa_insert(struct xarray *, uint32_t, void *, gfp_t);
void *xa_next(struct xarray *, unsigned long *, bool);

int __xa_alloc(struct xarray *, uint32_t *, void *, uint32_t, gfp_t);
int __xa_insert(struct xarray *, uint32_t, void *, gfp_t);
bool __xa_empty(struct xarray *);
void *__xa_next(struct xarray *, unsigned long *, bool);

/*
 * Furthur functions part of kernel drm graphics
 */
static inline int
xa_err(void *ptr)
{
	return (PTR_ERR_OR_ZERO(ptr));
}

static inline void *
xa_mk_value(unsigned long v)
{
	unsigned long r = (v << 1) | 1;

	return ((void *)r);
}

static inline bool
xa_is_value(const void *e)
{
	unsigned long v = (unsigned long)e;

	return (v & 1);
}

static inline unsigned long
xa_to_value(const void *e)
{
	unsigned long v = (unsigned long)e;

	return (v >> 1);
}

/*
 * OpenBSD additional API found in kernel drm graphics
 */

static inline bool
xa_is_err(const void *e)
{
	return xa_err(e) != 0;
}

void *xa_store_irq(struct xarray *xa, uint32_t index, void *entry, gfp_t gfp);
void *xa_erase_irq(struct xarray *xa, uint32_t index);

/*
 * OpenBSD implemented
 * not used in kernel drm grahics
 */
void xa_init(struct xarray *xa);

#endif

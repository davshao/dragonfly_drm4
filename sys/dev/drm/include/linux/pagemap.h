/* Public domain. */

/*
 * Copyright (c) 2015-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_PAGEMAP_H
#define _LINUX_PAGEMAP_H

// #include <linux/mm.h>
// #include <linux/fs.h>
// #include <linux/list.h>
// #include <linux/compiler.h>
#include <linux/uaccess.h>
#include <linux/highmem.h>
#include <linux/gfp.h>
// #include <linux/bitops.h>
// #include <linux/hardirq.h>

#if 0 /* not used in kernel drm */
static inline int
fault_in_pages_writeable(char __user *uaddr, int size)
{
	int ret = 0;
	char __user *end = uaddr + size - 1;

	if (unlikely(size == 0))
		return ret;

	/*
	 * Writing zeroes into userspace here is OK, because we know that if
	 * the zero gets there, we'll be overwriting it.
	 */
	while (uaddr <= end) {
		ret = subyte(uaddr, 0);
		if (ret != 0)
			return -EFAULT;
		uaddr += PAGE_SIZE;
	}

	/* Check whether the range spilled into the next page. */
	if (((unsigned long)uaddr & ~PAGE_MASK) ==
			((unsigned long)end & ~PAGE_MASK))
		ret = subyte(end, 0);

	return ret;
}

static inline int
fault_in_pages_readable(const char __user *uaddr, int size)
{
	char c;
	int ret = 0;
	const char __user *end = uaddr + size - 1;

	if (unlikely(size == 0))
		return ret;

	while (uaddr <= end) {
		ret = -copyin(uaddr, &c, 1);
		if (ret != 0)
			return -EFAULT;
		uaddr += PAGE_SIZE;
	}

	/* Check whether the range spilled into the next page. */
	if (((unsigned long)uaddr & ~PAGE_MASK) ==
			((unsigned long)end & ~PAGE_MASK)) {
		ret = -copyin(end, &c, 1);
	}

	return ret;
}
#endif

static inline gfp_t
mapping_gfp_mask(struct vm_object * mapping)
{
	/* XXX Linux mappings have an associated gfp_mask attribute */
	return GFP_KERNEL;
}

static inline gfp_t
mapping_gfp_constraint(struct vm_object *mapping, gfp_t gfp_mask)
{
	return (mapping_gfp_mask(mapping) & gfp_mask);
}

#endif

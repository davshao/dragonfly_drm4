/* Public domain. */

/*
 * Copyright (c) 2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_DMA_FENCE_ARRAY_H
#define _LINUX_DMA_FENCE_ARRAY_H

#include <linux/dma-fence.h>
#include <linux/irq_work.h>

struct dma_fence_array;

struct dma_fence_array_cb {
        struct dma_fence_cb cb;
        struct dma_fence_array *array;
};

struct dma_fence_array {
        struct dma_fence base;
        unsigned int num_fences;
        struct dma_fence **fences;

#if defined(__OpenBSD__)
	struct mutex lock;
	struct timeout to;
	int num_pending;
#else
        struct lock lock;
        struct irq_work work;
        atomic_t num_pending;
#endif
};

extern const struct dma_fence_ops dma_fence_array_ops;

static inline struct dma_fence_array *
to_dma_fence_array(struct dma_fence *fence)
{
        if (fence->ops != &dma_fence_array_ops)
                return NULL;

        return container_of(fence, struct dma_fence_array, base);
}

static inline bool
dma_fence_is_array(struct dma_fence *fence)
{
        return fence->ops == &dma_fence_array_ops;
}

struct dma_fence_array *dma_fence_array_create(int, struct dma_fence **,
    u64, unsigned, bool);

#endif

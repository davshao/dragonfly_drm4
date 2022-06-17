/* Public domain. */

/*
 * Copyright (c) 2019 Jonathan Gray <jsg@openbsd.org>
 * Copyright (c) 2020 François Tigeot <ftigeot@wolfpond.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _LINUX_DMA_FENCE_H
#define _LINUX_DMA_FENCE_H

#include <sys/types.h>
#include <sys/lock.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>
// #include <linux/err.h>
// #include <linux/wait.h>
// #include <linux/bitops.h>
// #include <linux/printk.h>

#define DMA_FENCE_TRACE(fence, fmt, args...) do {} while(0)

struct dma_fence_cb;

struct dma_fence {
	struct kref refcount;
	const struct dma_fence_ops *ops;
	unsigned long flags;
	uint64_t context;
#if defined(__OpenBSD__)
	uint64_t seqno;
	struct mutex *lock;
	union {
		struct list_head cb_list;
		ktime_t timestamp;
		struct rcu_head rcu;
	};
#else
	unsigned seqno;
	struct lock *lock;
	struct list_head cb_list;
	ktime_t timestamp;
	struct rcu_head rcu;
#endif
	int error;
};

enum dma_fence_flag_bits {
	DMA_FENCE_FLAG_SIGNALED_BIT,
	DMA_FENCE_FLAG_TIMESTAMP_BIT,
	DMA_FENCE_FLAG_ENABLE_SIGNAL_BIT,
	DMA_FENCE_FLAG_USER_BITS, /* must always be last member */
};

struct dma_fence_ops {
	const char * (*get_driver_name)(struct dma_fence *);
	const char * (*get_timeline_name)(struct dma_fence *);
	bool (*enable_signaling)(struct dma_fence *);
	bool (*signaled)(struct dma_fence *);
	long (*wait)(struct dma_fence *, bool, long);
	void (*release)(struct dma_fence *);

	int (*fill_driver_data)(struct dma_fence *fence, void *data, int size);
	void (*fence_value_str)(struct dma_fence *fence, char *str, int size);
	void (*timeline_value_str)(struct dma_fence *fence,
				   char *str, int size);
	bool use_64bit_seqno;
};

struct dma_fence_cb;
typedef void (*dma_fence_func_t)(struct dma_fence *fence, struct dma_fence_cb *cb);

struct dma_fence_cb {
	struct list_head node;
	dma_fence_func_t func;
};

uint64_t dma_fence_context_alloc(unsigned int);
struct dma_fence *dma_fence_get(struct dma_fence *);
struct dma_fence *dma_fence_get_rcu(struct dma_fence *);
struct dma_fence *dma_fence_get_rcu_safe(struct dma_fence **);
void dma_fence_release(struct kref *);
void dma_fence_put(struct dma_fence *);
int dma_fence_signal(struct dma_fence *);
int dma_fence_signal_locked(struct dma_fence *);
int dma_fence_signal_timestamp(struct dma_fence *, ktime_t);
int dma_fence_signal_timestamp_locked(struct dma_fence *, ktime_t);
bool dma_fence_is_signaled(struct dma_fence *);
bool dma_fence_is_signaled_locked(struct dma_fence *);
long dma_fence_default_wait(struct dma_fence *, bool, long);
long dma_fence_wait_any_timeout(struct dma_fence **, uint32_t, bool, long,
    uint32_t *);
long dma_fence_wait_timeout(struct dma_fence *, bool, long);
long dma_fence_wait(struct dma_fence *, bool);
void dma_fence_enable_sw_signaling(struct dma_fence *);
#if defined(__OpenBSD__)
void dma_fence_init(struct dma_fence *, const struct dma_fence_ops *,
    struct mutex *, uint64_t, uint64_t);
#else
void dma_fence_init(struct dma_fence *, const struct dma_fence_ops *,
    spinlock_t *lock, uint64_t, unsigned);
#endif
int dma_fence_add_callback(struct dma_fence *, struct dma_fence_cb *,
    dma_fence_func_t);
bool dma_fence_remove_callback(struct dma_fence *, struct dma_fence_cb *);

struct dma_fence *dma_fence_get_stub(void);
struct dma_fence *dma_fence_allocate_private_stub(void);

#if defined(__OpenBSD__)
static inline void
dma_fence_free(struct dma_fence *fence)
{
	free(fence, M_DRM, 0);
}
#else
void dma_fence_free(struct dma_fence *fence);
#endif

/*
 * is a later than b
 * if a and b are the same, should return false to avoid unwanted work
 */
static inline bool
__dma_fence_is_later(uint64_t a, uint64_t b, const struct dma_fence_ops *ops)
{
	uint32_t al, bl;

	if (ops->use_64bit_seqno)
		return a > b;

	al = a & 0xffffffff;
	bl = b & 0xffffffff;

	return (int)(al - bl) > 0;
}

static inline bool
dma_fence_is_later(struct dma_fence *a, struct dma_fence *b)
{
#if defined(__OpenBSD__)
	if (a->context != b->context)
		return false;
	return __dma_fence_is_later(a->seqno, b->seqno, a->ops);
#else
	return (a->seqno > b->seqno);
#endif
}

static inline void
dma_fence_set_error(struct dma_fence *fence, int error)
{
	fence->error = error;
}

#endif

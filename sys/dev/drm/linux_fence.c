/*
 * Copyright (c) 2019-2020 Jonathan Gray <jsg@openbsd.org>
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

/*	$OpenBSD: drm_linux.c,v 1.94 2022/09/16 01:48:07 jsg Exp $	*/
/*
 * Copyright (c) 2013 Jonathan Gray <jsg@openbsd.org>
 * Copyright (c) 2015, 2016 Mark Kettenis <kettenis@openbsd.org>
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

#include <linux/err.h>
#include <linux/slab.h>
#include <linux/dma-fence.h>


struct dma_fence *
dma_fence_get(struct dma_fence *fence)
{
	if (fence)
		kref_get(&fence->refcount);
	return fence;
}

struct dma_fence *
dma_fence_get_rcu(struct dma_fence *fence)
{
	if (fence)
		kref_get(&fence->refcount);
	return fence;
}

struct dma_fence *
dma_fence_get_rcu_safe(struct dma_fence **dfp)
{
	struct dma_fence *fence;
	if (dfp == NULL)
		return NULL;
	fence = *dfp;
	if (fence)
		kref_get(&fence->refcount);
	return fence;
}

void
dma_fence_release(struct kref *ref)
{
	struct dma_fence *fence = container_of(ref, struct dma_fence, refcount);

	if (fence->ops && fence->ops->release)
		fence->ops->release(fence);
	else
		kfree(fence);
}

void
dma_fence_put(struct dma_fence *fence)
{
	if (fence)
		kref_put(&fence->refcount, dma_fence_release);
}

int
dma_fence_signal_timestamp_locked(struct dma_fence *fence, ktime_t timestamp)
{
	struct dma_fence_cb *cur, *tmp;
	struct list_head cb_list;

	if (fence == NULL)
		return -EINVAL;

	if (test_and_set_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
		return -EINVAL;

	list_replace(&fence->cb_list, &cb_list);

	fence->timestamp = timestamp;
	set_bit(DMA_FENCE_FLAG_TIMESTAMP_BIT, &fence->flags);

	list_for_each_entry_safe(cur, tmp, &cb_list, node) {
		INIT_LIST_HEAD(&cur->node);
		cur->func(fence, cur);
	}

	return 0;
}

int
dma_fence_signal(struct dma_fence *fence)
{
	int r;

	if (fence == NULL)
		return -EINVAL;

	lockmgr(fence->lock, LK_EXCLUSIVE);
	r = dma_fence_signal_timestamp_locked(fence, ktime_get());
	lockmgr(fence->lock, LK_RELEASE);

	return r;
}

int
dma_fence_signal_locked(struct dma_fence *fence)
{
	if (fence == NULL)
		return -EINVAL;

	return dma_fence_signal_timestamp_locked(fence, ktime_get());
}

int
dma_fence_signal_timestamp(struct dma_fence *fence, ktime_t timestamp)
{
	int r;

	if (fence == NULL)
		return -EINVAL;

	lockmgr(fence->lock, LK_EXCLUSIVE);
	r = dma_fence_signal_timestamp_locked(fence, timestamp);
	lockmgr(fence->lock, LK_RELEASE);

	return r;
}

bool
dma_fence_is_signaled(struct dma_fence *fence)
{
	if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
		return true;

	if (fence->ops->signaled && fence->ops->signaled(fence)) {
		dma_fence_signal(fence);
		return true;
	}

	return false;
}

bool
dma_fence_is_signaled_locked(struct dma_fence *fence)
{
	if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
		return true;

	if (fence->ops->signaled && fence->ops->signaled(fence)) {
		dma_fence_signal_locked(fence);
		return true;
	}

	return false;
}

long
dma_fence_wait_timeout(struct dma_fence *fence, bool intr, long timeout)
{
	if (timeout < 0)
		return -EINVAL;

	if (fence->ops->wait)
		return fence->ops->wait(fence, intr, timeout);
	else
		return dma_fence_default_wait(fence, intr, timeout);
}

long
dma_fence_wait(struct dma_fence *fence, bool intr)
{
	long ret;

	ret = dma_fence_wait_timeout(fence, intr, MAX_SCHEDULE_TIMEOUT);
	if (ret < 0)
		return ret;
	
	return 0;
}

void
dma_fence_enable_sw_signaling(struct dma_fence *fence)
{
	if (!test_and_set_bit(DMA_FENCE_FLAG_ENABLE_SIGNAL_BIT, &fence->flags) &&
	    !test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags) &&
	    fence->ops->enable_signaling) {
		lockmgr(fence->lock, LK_EXCLUSIVE);
		if (!fence->ops->enable_signaling(fence))
			dma_fence_signal_locked(fence);
		lockmgr(fence->lock, LK_RELEASE);
	}
}

void
dma_fence_init(struct dma_fence *fence, const struct dma_fence_ops *ops,
    spinlock_t *lock, u64 context, unsigned seqno)
{
	fence->ops = ops;
	fence->lock = lock;
	fence->context = context;
	fence->seqno = seqno;
	fence->flags = 0;
	fence->error = 0;
	kref_init(&fence->refcount);
	INIT_LIST_HEAD(&fence->cb_list);
}

int
dma_fence_add_callback(struct dma_fence *fence, struct dma_fence_cb *cb,
    dma_fence_func_t func)
{
	int ret = 0;
	bool was_set;

	if (WARN_ON(!fence || !func))
		return -EINVAL;

	if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags)) {
		INIT_LIST_HEAD(&cb->node);
		return -ENOENT;
	}

	lockmgr(fence->lock, LK_EXCLUSIVE);

	was_set = test_and_set_bit(DMA_FENCE_FLAG_ENABLE_SIGNAL_BIT, &fence->flags);

	if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
		ret = -ENOENT;
	else if (!was_set && fence->ops->enable_signaling) {
		if (!fence->ops->enable_signaling(fence)) {
			dma_fence_signal_locked(fence);
			ret = -ENOENT;
		}
	}

	if (!ret) {
		cb->func = func;
		list_add_tail(&cb->node, &fence->cb_list);
	} else
		INIT_LIST_HEAD(&cb->node);
	lockmgr(fence->lock, LK_RELEASE);

	return ret;
}

bool
dma_fence_remove_callback(struct dma_fence *fence, struct dma_fence_cb *cb)
{
	bool ret;

	lockmgr(fence->lock, LK_EXCLUSIVE);

	ret = !list_empty(&cb->node);
	if (ret)
		list_del_init(&cb->node);

	lockmgr(fence->lock, LK_RELEASE);

	return ret;
}

#if defined(__OpenBSD__)
static atomic64_t drm_fence_context_count = ATOMIC64_INIT(1);
#else
static atomic64_t drm_fence_context_count = ATOMIC_INIT(1);
#endif

uint64_t
dma_fence_context_alloc(unsigned int num)
{
	return atomic64_add_return(num, &drm_fence_context_count) - num;
}

struct default_wait_cb {
	struct dma_fence_cb base;
	struct task_struct *task;
};

static void
dma_fence_default_wait_cb(struct dma_fence *fence, struct dma_fence_cb *cb)
{
	struct default_wait_cb *wait =
		container_of(cb, struct default_wait_cb, base);
	wake_up_process(wait->task);
}

long
dma_fence_default_wait(struct dma_fence *fence, bool intr, signed long timeout)
{
	long ret = timeout ? timeout : 1;
	unsigned long end;
	int err;
	struct default_wait_cb cb;
	bool was_set;

#if defined(__OpenBSD__)
	KASSERT(timeout <= INT_MAX);
#endif

	if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
		return ret;

	lockmgr(fence->lock, LK_EXCLUSIVE);

	was_set = test_and_set_bit(DMA_FENCE_FLAG_ENABLE_SIGNAL_BIT,
	    &fence->flags);

	if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
		goto out;

	if (!was_set && fence->ops->enable_signaling) {
		if (!fence->ops->enable_signaling(fence)) {
			dma_fence_signal_locked(fence);
			goto out;
		}
	}

	if (timeout == 0) {
		ret = 0;
		goto out;
	}

	cb.base.func = dma_fence_default_wait_cb;
	cb.task = current;
	list_add(&cb.base.node, &fence->cb_list);

	end = jiffies + timeout;
	for (ret = timeout; ret > 0; ret = MAX(0, end - jiffies)) {
		if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags)) {
			break;
		}
#if defined(__OpenBSD__)
		err = msleep(curproc, fence->lock, intr ? PCATCH : 0,
		    "dmafence", ret);
#else
		if (intr) {
			__set_current_state(TASK_INTERRUPTIBLE);
		}
		else {
			__set_current_state(TASK_UNINTERRUPTIBLE);
		}
		/* wake_up_process() directly uses task_struct pointers as sleep identifiers */
		err = lksleep(current, fence->lock, intr ? PCATCH : 0, "dmafence", ret);
#endif
		if (err == EINTR || err == ERESTART) {
			ret = -ERESTARTSYS;
			break;
		}
	}

	if (!list_empty(&cb.base.node))
		list_del(&cb.base.node);
#if defined(__DragonFly__)
	__set_current_state(TASK_RUNNING);
#endif
out:
	lockmgr(fence->lock, LK_RELEASE);

	return ret;
}

static bool
dma_fence_test_signaled_any(struct dma_fence **fences, uint32_t count,
    uint32_t *idx)
{
	int i;

	for (i = 0; i < count; ++i) {
		struct dma_fence *fence = fences[i];
		if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags)) {
			if (idx)
				*idx = i;
			return true;
		}
	}
	return false;
}

long
dma_fence_wait_any_timeout(struct dma_fence **fences, uint32_t count,
    bool intr, long timeout, uint32_t *idx)
{
	struct default_wait_cb *cb;
	long ret = timeout;
	unsigned long end;
	int i, err;

#if defined(__OpenBSD__)
	KASSERT(timeout <= INT_MAX);
#endif

	if (timeout == 0) {
		for (i = 0; i < count; i++) {
			if (dma_fence_is_signaled(fences[i])) {
				if (idx)
					*idx = i;
				return 1;
			}
		}
		return 0;
	}

#if defined(__OpenBSD__)
	cb = mallocarray(count, sizeof(*cb), M_DRM, M_WAITOK|M_CANFAIL|M_ZERO);
#else
	cb = kcalloc(count, sizeof(struct default_wait_cb), GFP_KERNEL);
#endif
	if (cb == NULL)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		struct dma_fence *fence = fences[i];
#if defined(__OpenBSD__)
		cb[i].proc = curproc;
#else
		cb[i].task = current;
#endif
		if (dma_fence_add_callback(fence, &cb[i].base,
		    dma_fence_default_wait_cb)) {
			if (idx)
				*idx = i;
			goto cb_cleanup;
		}
	}

	end = jiffies + timeout;
	for (ret = timeout; ret > 0; ret = MAX(0, end - jiffies)) {
		if (dma_fence_test_signaled_any(fences, count, idx))
			break;
#if defined(__OpenBSD__)
		err = tsleep(curproc, intr ? PCATCH : 0, "dfwat", ret);
#else
		err = tsleep(current, intr ? PCATCH : 0, "dfwat", ret);
#endif
		if (err == EINTR || err == ERESTART) {
			ret = -ERESTARTSYS;
			break;
		}
	}

cb_cleanup:
	while (i-- > 0)
		dma_fence_remove_callback(fences[i], &cb[i].base);
#if defined(__OpenBSD__)
	free(cb, M_DRM, count * sizeof(*cb));
#else
	kfree(cb);
#endif
	return ret;
}

static struct dma_fence dma_fence_stub;

#if defined(__OpenBSD__)
static struct mutex dma_fence_stub_mtx = MUTEX_INITIALIZER(IPL_TTY);
#else
static struct lock dma_fence_stub_mtx = LOCK_INITIALIZER("drmfst", 0, 0);
#endif

static const char *
dma_fence_stub_get_name(struct dma_fence *fence)
{
	return "stub";
}

static const struct dma_fence_ops dma_fence_stub_ops = {
	.get_driver_name = dma_fence_stub_get_name,
	.get_timeline_name = dma_fence_stub_get_name,
};

struct dma_fence *
dma_fence_get_stub(void)
{
	lockmgr(&dma_fence_stub_mtx, LK_EXCLUSIVE);
	if (dma_fence_stub.ops == NULL) {
		dma_fence_init(&dma_fence_stub, &dma_fence_stub_ops,
		    &dma_fence_stub_mtx, 0, 0);
		dma_fence_signal_locked(&dma_fence_stub);
	}
	lockmgr(&dma_fence_stub_mtx, LK_RELEASE);

	return dma_fence_get(&dma_fence_stub);
}

struct dma_fence *
dma_fence_allocate_private_stub(void)
{
	struct dma_fence *f = kzalloc(sizeof(*f),
	    GFP_KERNEL);
	if (f == NULL)
		return ERR_PTR(-ENOMEM);
	dma_fence_init(f, &dma_fence_stub_ops, &dma_fence_stub_mtx, 0, 0);
	dma_fence_signal(f);
	return f;
}

void
dma_fence_free(struct dma_fence *fence)
{
	kfree(fence);
}

#include <linux/dma-fence-array.h>

static const char *
dma_fence_array_get_driver_name(struct dma_fence *fence)
{
	return "dma_fence_array";
}

static const char *
dma_fence_array_get_timeline_name(struct dma_fence *fence)
{
	return "unbound";
}

static void
#if defined(__OpenBSD__)
irq_dma_fence_array_work(void *arg)
#else
irq_dma_fence_array_work(struct irq_work *wrk)
#endif
{
#if defined(__OpenBSD__)
	struct dma_fence_array *dfa = (struct dma_fence_array *)arg;
#else
	struct dma_fence_array *dfa = container_of(wrk, typeof(*dfa), work);
#endif

	dma_fence_signal(&dfa->base);
	dma_fence_put(&dfa->base);
}

static void
dma_fence_array_cb_func(struct dma_fence *f, struct dma_fence_cb *cb)
{
	struct dma_fence_array_cb *array_cb =
	    container_of(cb, struct dma_fence_array_cb, cb);
	struct dma_fence_array *dfa = array_cb->array;
	
	if (atomic_dec_and_test(&dfa->num_pending))
#if defined(__OpenBSD__)
		timeout_add(&dfa->to, 1);
#else
		irq_work_queue(&dfa->work);
#endif
	else
		dma_fence_put(&dfa->base);
}

static bool
dma_fence_array_enable_signaling(struct dma_fence *fence)
{
	struct dma_fence_array *dfa = to_dma_fence_array(fence);
	struct dma_fence_array_cb *cb = (void *)(&dfa[1]);
	int i;

	for (i = 0; i < dfa->num_fences; ++i) {
		cb[i].array = dfa;
		dma_fence_get(&dfa->base);
		if (dma_fence_add_callback(dfa->fences[i], &cb[i].cb,
		    dma_fence_array_cb_func)) {
			dma_fence_put(&dfa->base);
			if (atomic_dec_and_test(&dfa->num_pending))
				return false;
		}
	}
	
	return true;
}

static bool
dma_fence_array_signaled(struct dma_fence *fence)
{
	struct dma_fence_array *dfa = to_dma_fence_array(fence);

	return atomic_read(&dfa->num_pending) <= 0;
}

static void
dma_fence_array_release(struct dma_fence *fence)
{
	struct dma_fence_array *dfa = to_dma_fence_array(fence);
	int i;

	for (i = 0; i < dfa->num_fences; ++i)
		dma_fence_put(dfa->fences[i]);

#if defined(__OpenBSD__)
	free(dfa->fences, M_DRM, 0);
#else
	_kfree(dfa->fences, M_DRM);
#endif
	dma_fence_free(fence);
}

const struct dma_fence_ops dma_fence_array_ops = {
	.get_driver_name = dma_fence_array_get_driver_name,
	.get_timeline_name = dma_fence_array_get_timeline_name,
	.enable_signaling = dma_fence_array_enable_signaling,
	.signaled = dma_fence_array_signaled,
	.wait = dma_fence_default_wait,
	.release = dma_fence_array_release,
};

struct dma_fence_array *
dma_fence_array_create(int num_fences, struct dma_fence **fences, u64 context,
    unsigned seqno, bool signal_on_any)
{
#if defined(__OpenBSD__)
	struct dma_fence_array *dfa = malloc(sizeof(*dfa) +
	    (num_fences * sizeof(struct dma_fence_array_cb)),
	    M_DRM, M_WAITOK|M_CANFAIL|M_ZERO);
#else
	struct dma_fence_array *dfa = kzalloc(sizeof(*dfa) +
	    (num_fences * sizeof(struct dma_fence_array_cb)),
	    GFP_KERNEL);
#endif
	if (dfa == NULL)
		return NULL;

	lockinit(&dfa->lock, "ldmbfal", 0, 0);
	dma_fence_init(&dfa->base, &dma_fence_array_ops, &dfa->lock,
	    context, seqno);
#if defined(__OpenBSD__)
	timeout_set(&dfa->to, irq_dma_fence_array_work, dfa);
#else
	init_irq_work(&dfa->work, irq_dma_fence_array_work);
#endif

	dfa->num_fences = num_fences;
	atomic_set(&dfa->num_pending, signal_on_any ? 1 : num_fences);
	dfa->fences = fences;

	return dfa;
}

#include <linux/dma-fence-chain.h>

struct dma_fence_chain *
dma_fence_chain_alloc(void)
{
#if defined(__OpenBSD__)
	return malloc(sizeof(struct dma_fence_chain), M_DRM,
	    M_WAITOK | M_CANFAIL);
#else
	return __kmalloc(sizeof(struct dma_fence_chain), M_DRM,
	    M_WAITOK | M_NULLOK);
#endif
}

void
dma_fence_chain_free(struct dma_fence_chain *dfc)
{
#if defined(__OpenBSD__)
	free(dfc, M_DRM, sizeof(struct dma_fence_chain));
#else
	_kfree(dfc, M_DRM);
#endif
}


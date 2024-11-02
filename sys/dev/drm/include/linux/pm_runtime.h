/* Public domain. */

/*
 * Copyright (c) 2017-2019 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_PM_RUNTIME_H
#define _LINUX_PM_RUNTIME_H

#include <sys/types.h>
#if defined(__OpenBSD__)
#include <sys/device.h>
#else
#include <linux/device.h>
#endif
// #include <linux/notifier.h>
#include <linux/pm.h>
// #include <linux/jiffies.h>

extern void pm_runtime_enable(struct device *dev);

static inline void
pm_runtime_mark_last_busy(struct device *dev)
{
	/* pm_runtime_mark_last_busy not implemented */
}

static inline void
pm_runtime_use_autosuspend(struct device *dev)
{
	/* pm_runtime_use_autosuspend not implemented */
}
 
static inline void
pm_runtime_dont_use_autosuspend(struct device *dev)
{
	/* pm_runtime_dont_use_autosuspend not implemented */
}

static inline void
pm_runtime_put_autosuspend(struct device *dev)
{
	/* pm_runtime_put_autosuspend not implemented */
}

static inline void
pm_runtime_set_autosuspend_delay(struct device *dev, int x)
{
	/* pm_runtime_set_autosuspend_delay not implemented */
}

static inline int
pm_runtime_set_active(struct device *dev)
{
	/* pm_runtime_set_active not implemented */
	return 0;
}

static inline void
pm_runtime_allow(struct device *dev)
{
	/* pm_runtime_allow not implemented */
}

static inline void
pm_runtime_put_noidle(struct device *dev)
{
	/* pm_runtime_put_noidle implemented */
}

static inline void
pm_runtime_forbid(struct device *dev)
{
	/* pm_runtime_forbid not implemented */
}

static inline void
pm_runtime_get_noresume(struct device *dev)
{
	/* pm_runtime_get_noresume not implemented */
}

static inline void
pm_runtime_put(struct device *dev)
{
	/* pm_runtime_put not implemented */
}

static inline int
pm_runtime_get_sync(struct device *dev)
{
	/* pm_runtime_get_sync not implemented */
	return 0;
}

static inline int
pm_runtime_get_if_in_use(struct device *dev)
{
	/* pm_runtime_get_if_in_use not implemented */
	return -EINVAL;
}

static inline int
pm_runtime_get_if_active(struct device *dev, bool x)
{
	/* pm_runtime_get_if_active not implemented */
	return -EINVAL;
}

static inline void
pm_runtime_disable(struct device *dev)
{
	/* pm_runtime_disable not implemented */
}

#endif

/* Public domain. */

/*
 * Copyright (c) 2014-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_DEVICE_H
#define _LINUX_DEVICE_H

#include <sys/types.h>
#include <sys/systm.h>
#if defined(__OpenBSD__)
#include <sys/device.h>
#include <sys/param.h>
#include <sys/proc.h>
#else
#include <sys/bus.h> /* defines device_printf() */
#endif
// #include <linux/slab.h>
#include <linux/ioport.h>
// #include <linux/list.h>
#include <linux/lockdep.h>
// #include <linux/compiler.h>
// #include <linux/types.h>
// #include <linux/mutex.h>
#include <linux/pm.h>
#include <linux/kobject.h>
#include <linux/ratelimit.h> /* dev_printk.h -> ratelimit.h */
// #include <linux/atomic.h>
// #include <linux/gfp.h>

/* OpenBSD defines struct device in sys/device.h */
struct device {
	struct device	*parent;

	struct kobject kobj;

	device_t	bsddev;

	void		*driver_data;	/* for dev_set and get_drvdata */

	struct dev_pm_info power;
};

struct device_node;

struct device_driver {
	struct device *dev;
	const struct dev_pm_ops *pm;
};

struct device_attribute {
	struct attribute attr;
	ssize_t (*show)(struct device *, struct device_attribute *, char *);
};

#define DEVICE_ATTR(_name, _mode, _show, _store) \
	struct device_attribute dev_attr_##_name
#define DEVICE_ATTR_RO(_name) \
	struct device_attribute dev_attr_##_name

#define device_create_file(a, b)	0
#define device_remove_file(a, b)

#if defined(__OpenBSD__)
#define dev_get_drvdata(x)	NULL
#define dev_set_drvdata(x, y)
#else
static inline void
dev_set_drvdata(struct device *dev, void *data)
{
	dev->driver_data = data;
}

static inline void *
dev_get_drvdata(const struct device *dev)
{
	return dev->driver_data;
}
#endif

#define dev_pm_set_driver_flags(x, y)

#if defined(__OpenBSD__)
#define devm_kzalloc(x, y, z)	kzalloc(y, z)
#else
#define devm_kzalloc(x, y, z)	__kmalloc(y, M_DRM, z | M_ZERO)
#endif

#ifndef DRM_CURPROCPID
#define DRM_CURPROCPID		(curproc != NULL ? curproc->p_pid : -1)
#endif

#if defined(__DragonFly__) /* previous DragonFly */
#define	dev_warn(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *WARNING*: " fmt, \
	    __func__, ## arg)
#define dev_notice(dev, fmt, arg...)					\
	device_printf((dev)->bsddev, "drm: %s *NOTICE*: " fmt,  \
	    __func__, ## arg)
#define	dev_crit(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *ERROR*: " fmt,  \
	    __func__, ## arg)
#define	dev_err(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *ERROR*: " fmt,  \
	    __func__, ## arg)
#define dev_emerg(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *EMERGENCY*: " fmt,  \
	    __func__, ## arg)
#else
#define	dev_warn(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *WARNING*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#define dev_notice(dev, fmt, arg...)					\
	device_printf((dev)->bsddev, "drm:pid%d:%s *NOTICE*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#define	dev_crit(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *ERROR*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#define	dev_err(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *ERROR*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#define dev_emerg(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *EMERGENCY*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#endif
#if defined(__OpenBSD__)
#define dev_printk(level, dev, fmt, arg...)				\
	printf("drm:pid%d:%s *PRINTK* " fmt, curproc->p_p->ps_pid,	\
	    __func__ , ## arg)
#else
static inline void
dev_printk(const char *level, const struct device *dev, const char *fmt, ...)
{
	__va_list ap;

	device_printf((dev)->bsddev, "%s: ", level);
	__va_start(ap, fmt);
	kprintf(fmt, ap);
	__va_end(ap);
}
#endif

#if defined(__DragonFly__) /* previous DragonFly */
#define dev_warn_ratelimited(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *WARNING*: " fmt,  \
	    __func__, ## arg)
#define dev_notice_ratelimited(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *NOTICE*: " fmt,  \
	    __func__, ## arg)
#define dev_err_ratelimited(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *ERROR*: " fmt,  \
	    __func__, ## arg)

#define dev_warn_once(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *WARNING*: " fmt,  \
	    __func__, ## arg)
#define dev_err_once(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: %s *ERROR*: " fmt,  \
	    __func__, ## arg)
#else
#define dev_warn_ratelimited(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *WARNING*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#define dev_notice_ratelimited(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *NOTICE*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#define dev_err_ratelimited(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *ERROR*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)

#define dev_warn_once(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *WARNING*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#define dev_err_once(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *ERROR*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#endif

#define dev_info(dev, fmt, arg...)						\
	device_printf(((struct device *)(dev))->bsddev, "drm: *INFO*: " fmt, ## arg)
#define dev_info_once(dev, fmt, arg...)						\
	device_printf(((struct device *)(dev))->bsddev, "drm: *INFO*: " fmt, ## arg)
#if defined(__DragonFly__)
#define dev_dbg(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: *DEBUG*: " fmt, ## arg)
#define dev_dbg_ratelimited(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm: *DEBUG*: " fmt, ## arg)
#else
#define dev_dbg(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *DEBUG*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#define dev_dbg_ratelimited(dev, fmt, arg...)						\
	device_printf((dev)->bsddev, "drm:pid%d:%s *DEBUG*: " fmt, DRM_CURPROCPID, \
	    __func__, ## arg)
#endif

static inline const char *
dev_driver_string(struct device *dev)
{
	return device_get_name((dev)->bsddev);
}

static inline const char *
dev_name(const struct device *dev)
{
	return("dev_name");
}

static inline int
dev_set_name(struct device *dev, const char *name, ...)
{
	return 0;
}

#endif

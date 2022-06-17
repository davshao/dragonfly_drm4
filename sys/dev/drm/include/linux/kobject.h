/* Public domain. */

/*
 * Copyright (c) 2016-2018 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_KOBJECT_H
#define _LINUX_KOBJECT_H

#include <sys/stdarg.h>

// #include <linux/types.h>
// #include <linux/list.h>
// #include <linux/compiler.h>
// #include <linux/spinlock.h>
#include <linux/kref.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
// #include <linux/wait.h>
// #include <linux/atomic.h>

enum kobject_action {
	KOBJ_ADD,
	KOBJ_REMOVE,
	KOBJ_CHANGE,
	KOBJ_MOVE,
	KOBJ_ONLINE,
	KOBJ_OFFLINE,
	KOBJ_MAX
};

struct kobject {
	struct kref kref;
	struct kobj_type *type;
	const char *name;
};

struct kobj_type {
	void (*release)(struct kobject *);
	const struct sysfs_ops *sysfs_ops;
	struct attribute **default_attrs;
};

struct kobj_attribute {
};

static inline void
kobject_init(struct kobject *obj, struct kobj_type *type)
{
	kref_init(&obj->kref);
	obj->type = type;
}

static inline int
kobject_init_and_add(struct kobject *obj, struct kobj_type *type,
    struct kobject *parent, const char *fmt, ...)
{
	va_list ap;

	kobject_init(obj, type);

	va_start(ap, fmt);
	obj->name = kvasprintf(M_WAITOK, fmt, ap);
	va_end(ap);

	return (0);
}

static inline struct kobject *
kobject_get(struct kobject *obj)
{
	if (obj != NULL)
		kref_get(&obj->kref);
	return (obj);
}

static inline void
kobject_release(struct kref *ref)
{
	struct kobject *obj = container_of(ref, struct kobject, kref);
	if (obj->type && obj->type->release)
		obj->type->release(obj);
}

static inline void
kobject_put(struct kobject *obj)
{
	if (obj != NULL)
		kref_put(&obj->kref, kobject_release);
}

static inline void
kobject_del(struct kobject *obj)
{
	/*
	   This function is supposed to unlink the object from a hierarchy.
	   There is no such hierarchy in the DragonFly implementation, doing
	   nothing is fine.
	*/
}

#if defined(__OpenBSD__)
#define kobject_uevent_env(obj, act, envp)
#else
static inline int
kobject_uevent_env(struct kobject *kobj, enum kobject_action action, char *envp[])
{
	return 0;
}
#endif

#if 0
extern __printf(4, 5) __must_check
int kobject_init_and_add(struct kobject *kobj,
			 struct kobj_type *ktype, struct kobject *parent,
			 const char *fmt, ...);

extern void kobject_release(struct kref *kref);
#endif

#endif

/* Public domain. */

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

#ifndef _LINUX_SYSFS_H
#define _LINUX_SYSFS_H

#include <sys/types.h>
// #include <linux/compiler.h>
// #include <linux/errno.h>
// #include <linux/list.h>
// #include <linux/lockdep.h>
#include <linux/atomic.h>
// #include <linux/kernfs.h>

struct kobject;

struct attribute {
	const char *name;
	umode_t	mode;
};

/* Code probably commented out for this as well */
struct bin_attribute {
};

/* OpenBSD comments out code using this */
struct sysfs_ops {
	ssize_t	(*show)(struct kobject *, struct attribute *,char *);
	ssize_t	(*store)(struct kobject *,struct attribute *,const char *, size_t);
};

struct attribute_group {
	const char *name;
	struct attribute **attrs;
	struct bin_attribute **bin_attrs;
};

static inline int
sysfs_create_link(struct kobject *kobj, struct kobject *target, const char *name)
{
	return 0;
}

static inline void
sysfs_remove_link(struct kobject *kobj, const char *name)
{
}

#if defined(__OpenBSD__)
#define sysfs_create_link(x, y, z)	0
#define sysfs_remove_link(x, y)
#endif
#define sysfs_create_group(x, y)	0
#define sysfs_remove_group(x, y)
#define sysfs_remove_file(x, y)
#define sysfs_remove_file_from_group(x, y, z)
#define sysfs_create_files(x, y)	0
#define sysfs_remove_files(x, y)

static inline int
sysfs_emit(char *str, const char *format, ...)
{
	return 0;
}

static inline int
sysfs_emit_at(char *str, int pos, const char *format, ...)
{
	return 0;
}

#endif

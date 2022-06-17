/* Public domain. */

/*
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013, 2014 Mellanox Technologies, Ltd.
 * Copyright (c) 2016-2019 François Tigeot <ftigeot@wolfpond.org>
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
#ifndef	_LINUX_MODULE_H
#define	_LINUX_MODULE_H

#include <sys/module.h>

#include <linux/export.h>
#include <linux/moduleparam.h>
#include <linux/kobject.h>

#include <linux/jump_label.h> /* DEFINE_STATIC_KEY_FALSE */
#include <linux/stat.h> /* S_IRUGO */

#if 0
#include <linux/list.h>
#include <linux/stat.h>
#include <linux/compiler.h>
#include <linux/cache.h>
// #include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/stddef.h>
#include <linux/errno.h>
#include <linux/workqueue.h>

#endif

#define MODULE_AUTHOR(x)
#define MODULE_DESCRIPTION(x)

#ifndef MODULE_LICENSE
#define MODULE_LICENSE(x)
#endif

#define MODULE_FIRMWARE(x)
#define MODULE_DEVICE_TABLE(x, y)

#ifndef MODULE_VERSION
#define MODULE_VERSION(name)
#endif

#if defined(__OpenBSD__)
#define module_init(x)
#else
#define module_init(fname)	\
	SYSINIT(fname, SI_SUB_DRIVERS, SI_ORDER_FIRST, fname, NULL);
#endif

#if defined(__OpenBSD__)
#define module_exit(x)
#else
#define module_exit(fname)	\
	SYSUNINIT(fname, SI_SUB_DRIVERS, SI_ORDER_SECOND, fname, NULL);
#endif

#define symbol_put(x)

static inline bool
try_module_get(struct module *m)
{
	return true;
}

static inline void
module_put(struct module *m)
{
}

#ifndef THIS_MODULE
struct module;
#define	THIS_MODULE	((struct module *)0)
#endif

#endif

/* Public domain. */

/*
 * Copyright (c) 2015 François Tigeot
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

#ifndef _LINUX_BACKLIGHT_H
#define _LINUX_BACKLIGHT_H

#include <linux/device.h>
#include <linux/fb.h>
#include <linux/mutex.h>
#include <linux/notifier.h>

struct backlight_device;

enum backlight_type {
	BACKLIGHT_RAW = 1,
	BACKLIGHT_PLATFORM,
	BACKLIGHT_FIRMWARE,
	BACKLIGHT_TYPE_MAX,
};

struct backlight_properties {
	enum backlight_type type;
	int max_brightness;
	int brightness;
	int power;
};

struct backlight_ops {
	int options;
	int (*update_status)(struct backlight_device *);
	int (*get_brightness)(struct backlight_device *);
};

struct backlight_device {
	const struct backlight_ops *ops;
	struct backlight_properties props;
#if defined(__OpenBSD__)
	struct task task;
#endif
	void *data;
};

static inline struct backlight_device *
devm_of_find_backlight(struct device *dev)
{
	return NULL;
}

static inline struct backlight_device *
backlight_device_get_by_name(const char *name)
{
	return NULL;
}

#endif

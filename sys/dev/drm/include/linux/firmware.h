/* Public domain. */

/*
 * Copyright (c) 2015 Michael Neumann
 * Copyright (c) 2016 François Tigeot
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

#ifndef _LINUX_FIRMWARE_H
#define _LINUX_FIRMWARE_H

#if defined(__OpenBSD__)
#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/device.h>
#else
#include <sys/bus.h>
#include <sys/firmware.h>
#endif
#include <linux/types.h>
#include <linux/gfp.h>

// #include <linux/compiler.h>
// #include <linux/device.h>

#if defined(__OpenBSD__)
#ifndef __DECONST
#define __DECONST(type, var)	((type)(__uintptr_t)(const void *)(var))
#endif

struct firmware {
	size_t size;
	const u8 *data;
};
#else
/* DragonFly __DECONST defined in sys/cdefs.h,
 * included in sys/bus.h */
struct device;
#endif

static inline int
request_firmware(const struct firmware **fw, const char *name,
    struct device *device)
{
#if defined(__OpenBSD__)
	int r;
	struct firmware *f = malloc(sizeof(struct firmware), M_DRM,
	    M_WAITOK | M_ZERO);
	r = loadfirmware(name, __DECONST(u_char **, &f->data), &f->size);
	if (r != 0) {
		free(f, M_DRM, sizeof(struct firmware));
		*fw = NULL;
		return -r;
	} else  {
		*fw = f;
		return 0;
	}
#else
	*fw = firmware_get(name);
	if (*fw) {
		return 0;
	}
	return -ENOENT;
#endif
}

static inline int
request_firmware_direct(const struct firmware **fw, const char *name,
    struct device *device)
{
	return request_firmware(fw, name, device);
}

#define request_firmware_nowait(a, b, c, d, e, f, g) -EINVAL

#if 0
/* Function never used in drm code only in FreeBSD linuxkpi */
static inline int
request_firmware_nowait(struct module *module, bool uevent,
    const char *name, struct device *device, gfp_t gfp, void *context,
    void (*cont)(const struct firmware *fw, void *context))
{
	const struct firmware *fw;

	fw = firmware_get(name);
	if (fw == NULL) {
		return -ENOENT;
	}

	cont(fw, context);

	return 0;
}
#endif

static inline void
release_firmware(const struct firmware *fw)
{
#if defined(__OpenBSD__)
	if (fw)
		free(__DECONST(u_char *, fw->data), M_DEVBUF, fw->size);
	free(__DECONST(struct firmware *, fw), M_DRM, sizeof(*fw));
#else
	if (fw) {
		firmware_put(fw, FIRMWARE_UNLOAD);
	}
#endif
}

#endif

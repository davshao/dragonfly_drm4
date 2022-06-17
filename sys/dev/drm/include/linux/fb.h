/* Public domain. */

/*
 * Copyright (c) 2015-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_FB_H
#define _LINUX_FB_H


#include <uapi/linux/fb.h>

#include <sys/types.h>
#if defined(__OpenBSD__)
#include <linux/slab.h>
#else
#include <linux/workqueue.h>
#endif
#include <linux/notifier.h>
#include <linux/backlight.h>
#include <linux/kgdb.h>
#include <linux/fs.h>
// #include <linux/init.h>
// #include <linux/list.h>
// #include <asm/io.h>

#include <machine/framebuffer.h>

#if defined(__OpenBSD__)
struct fb_cmap;
struct fb_fillrect;
struct fb_copyarea;
struct fb_image;
#endif

struct fb_info;

struct apertures_struct;

#if defined(__OpenBSD__)
struct fb_var_screeninfo {
	int pixclock;
	uint32_t width;
	uint32_t height;
};
#else
/* DragonFly definex in uapi/linux/fb.h */
#endif

#if defined(__OpenBSD__)
struct fb_ops {
	int (*fb_set_par)(struct fb_info *);
};

struct fb_info {
	struct fb_var_screeninfo var;
	const struct fb_ops *fbops;
	char *screen_buffer;
	void *par;
	int fbcon_rotate_hint;
	bool skip_vt_switch;
	int flags;
};
#else
/* DragonFly defines above in machine/framebuffer.h */
#endif

#define	KHZ2PICOS(a)	(1000000000UL/(a))

#if defined(__OpenBSD__)

#else /* defined in uapi/linux/fb.h */
#endif

#if defined(__OpenBSD__)
#define FB_BLANK_UNBLANK	0
#endif
#define FB_BLANK_NORMAL		1
#define FB_BLANK_HSYNC_SUSPEND	2
#define FB_BLANK_VSYNC_SUSPEND	3
#if defined(__OpenBSD__)
#define FB_BLANK_POWERDOWN	4
#endif

#define FBINFO_STATE_RUNNING	0
#define FBINFO_STATE_SUSPENDED	1

#define FBINFO_HIDE_SMEM_START	0

#define FB_ROTATE_UR		0
#define FB_ROTATE_CW		1
#define FB_ROTATE_UD		2
#define FB_ROTATE_CCW		3

struct videomode;

#if defined(__OpenBSD__)
static inline struct fb_info *
framebuffer_alloc(size_t size, void *dev)
{
	return kzalloc(sizeof(struct fb_info) + size, GFP_KERNEL);
}
#else
/* DragonFly defined in sys/dev/drm/linux_compat.c */
struct fb_info *
framebuffer_alloc(size_t size, struct device *dev);
#endif

#if defined(__OpenBSD__) || defined(__DragonFly__)
static inline void
fb_set_suspend(struct fb_info *fbi, int s)
{
}
#else /* Not sure where the implementation is */
void
fb_set_suspend(struct fb_info *fbi, int s);
#endif

#if defined(__OpenBSD__)
static inline void
framebuffer_release(struct fb_info *fbi)
{
	kfree(fbi);
}
#else
void
framebuffer_release(struct fb_info *fbi);
#endif

#if defined(__OpenBSD__)
static inline int
fb_get_options(const char *name, char **opt)
{
	return 0;
}
#else
int
fb_get_options(const char *name, char **opt);
#endif

#if defined(__OpenBSD__)
static inline int
register_framebuffer(struct fb_info *fbi)
{
	if (fbi->fbops && fbi->fbops->fb_set_par)
		fbi->fbops->fb_set_par(fbi);
	return 0;
}

static inline void
unregister_framebuffer(struct fb_info *fbi)
{
}
#else
/* DragonFly defines in sys/dev/misc/syscons/syscons.c */
#endif

struct apertures_struct {
	unsigned int count;
	struct aperture {
		resource_size_t base;
		resource_size_t size;
	} ranges[0];
};

#if 0
extern int remove_conflicting_framebuffers(struct apertures_struct *a,
					   const char *name, bool primary);
#endif

#define FBINFO_STATE_RUNNING	0

struct device_node;

#endif

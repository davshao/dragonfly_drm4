/* Public domain. */

/*
 * Copyright (c) 2015-2018 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_SWAP_H
#define _LINUX_SWAP_H

#if 0
#include <linux/spinlock.h>
#include <linux/mmzone.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <asm/page.h>
#endif

#if defined(__OpenBSD__)
#include <uvm/uvm_extern.h>
#else
/* from vm/swap_pager.h */
#include <sys/conf.h>
#include <vm/vm_page2.h>
#endif

#include <linux/mm_types.h>

extern int nswdev;
extern struct swdevt *swdevt;

static inline long
get_nr_swap_pages(void)
{
#if defined(__OpenBSD__)
	return uvmexp.swpages - uvmexp.swpginuse;
#else
	int n;
	struct swdevt *sp;
	long total_pages = 0;
	long used_pages = 0;

	for (n = 0; n < nswdev; n++) {
		sp = &swdevt[n];

		total_pages += sp->sw_nblks;
		used_pages  += sp->sw_nused;
	}

	return (total_pages - used_pages);
#endif
}

/* 
 * XXX For now, we don't want the shrinker to be too aggressive, so
 * pretend we're not called from the pagedaemon even if we are.
 */
static inline int
current_is_kswapd(void)
{
	return 0;
}

static inline void
mark_page_accessed(struct page *m)
{
	vm_page_flag_set((struct vm_page *)m, PG_REFERENCED);
}

#endif

/* Public domain. */

/*
 * Copyright (c) 2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_PAGEVEC_H
#define _LINUX_PAGEVEC_H

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/errno.h>

// #include <linux/mm.h> for put_page()

#define PAGEVEC_SIZE	15

struct page;

struct pagevec {
	unsigned char nr;
	bool pv_unused0;
	struct page *pages[PAGEVEC_SIZE];
};

#if defined(__OpenBSD__)
void __pagevec_release(struct pagevec *);

/* From drm_linux.c */
void
__pagevec_release(struct pagevec *pvec)
{
	struct pglist mlist;
	int i;

	TAILQ_INIT(&mlist);
	for (i = 0; i < pvec->nr; i++)
		TAILQ_INSERT_TAIL(&mlist, pvec->pages[i], pageq);
	uvm_pglistfree(&mlist);
	pagevec_reinit(pvec);
}
#else
static inline void
__pagevec_release(struct pagevec *pvec)
{
	for (int i = 0; i < pvec->nr; i++)
		put_page(pvec->pages[i]);

	pvec->nr = 0;
}
#endif

#if defined(__OpenBSD__)
static inline unsigned int
pagevec_space(struct pagevec *pvec)
{
	return PAGEVEC_SIZE - pvec->nr;
}
#else
static inline unsigned int
pagevec_space(struct pagevec *pvec)
{
	return PAGEVEC_SIZE - pvec->nr;
}
#endif

static inline void
pagevec_init(struct pagevec *pvec)
{
	pvec->nr = 0;
}

static inline void
pagevec_reinit(struct pagevec *pvec)
{
	pvec->nr = 0;
}

static inline unsigned int
pagevec_count(struct pagevec *pvec)
{
	return pvec->nr;
}

#if defined(__OpenBSD__)
static inline unsigned int
pagevec_add(struct pagevec *pvec, struct vm_page *page)
{
	pvec->pages[pvec->nr++] = page;
	return PAGEVEC_SIZE - pvec->nr;
}
#else
static inline unsigned int
pagevec_add(struct pagevec *pvec, struct page *page)
{
	pvec->pages[pvec->nr] = page;
	pvec->nr++;

	return (PAGEVEC_SIZE - pvec->nr);
}
#endif

#endif

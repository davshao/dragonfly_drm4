/*	$OpenBSD: hashtable.h,v 1.2 2020/06/08 04:48:14 jsg Exp $	*/
/*
 * Copyright (c) 2017 Mark Kettenis
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*	$NetBSD: hashtable.h,v 1.5 2018/08/27 06:39:27 riastradh Exp $	*/

/*-
 * Copyright (c) 2018 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Taylor R. Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LINUX_HASHTABLE_H
#define _LINUX_HASHTABLE_H

#if defined(__DragonFly__)
#include <sys/param.h>
#include <sys/types.h>
#endif

#include <linux/list.h>
// #include <linux/types.h>
// #include <linux/kernel.h>
#include <linux/hash.h>
// #include <linux/rculist.h>

#define DECLARE_HASHTABLE(name, bits) struct hlist_head name[1 << (bits)]

static inline void
__hash_init(struct hlist_head *table, u_int size)
{
	u_int i;

	for (i = 0; i < size; i++)
		INIT_HLIST_HEAD(&table[i]);
}

static inline bool
__hash_empty(struct hlist_head *table, u_int size)
{
	u_int i;

	for (i = 0; i < size; i++) {
		if (!hlist_empty(&table[i]))
			return false;
	}

	return true;
}

#define	__arraycount(array)	NELEM(array)

#if defined(__OpenBSD__) /* question why nitems(table) - 1? */
#define __hash(table, key)	&table[key % (nitems(table) - 1)]
#else
#define __hash(table, key)	&table[key % (nitems(table))]
#endif

#if defined(__OpenBSD__)
#define hash_init(table)	__hash_init(table, nitems(table))
#else
#define hash_init(table)	__hash_init((table), __arraycount(table))
#endif
#if defined(__OpenBSD__)
#define hash_add(table, node, key) \
	hlist_add_head(node, __hash(table, key))
#else
#define hash_add(table, node, key) \
	hlist_add_head(node, &(table)[(key) % __arraycount(table)])
#endif
#define hash_del(node)		hlist_del_init(node)
#if defined(__OpenBSD__)
#define hash_empty(table)	__hash_empty(table, nitems(table))
#else
#define hash_empty(table)	__hash_empty((table), __arraycount(table))
#endif

#if defined(__OpenBSD__)
#define hash_for_each_possible(table, obj, member, key) \
	hlist_for_each_entry(obj, __hash(table, key), member)
#else
#define	hash_for_each_possible(table, obj, member, key) \
	hlist_for_each_entry(obj, &(table)[(key) % __arraycount(table)], member)
#endif

#if defined(__OpenBSD__)
#define hash_for_each_safe(table, i, tmp, obj, member) 	\
	for (i = 0; i < nitems(table); i++)		\
	       hlist_for_each_entry_safe(obj, tmp, &table[i], member)
#else
#define	hash_for_each_safe(table, i, tmp, obj, member)  \
	for ((i) = 0; (i) < __arraycount(table); (i)++) \
	       hlist_for_each_entry_safe(obj, tmp, &(table)[i], member)
#endif

#endif

/*	$OpenBSD: list.h,v 1.5 2022/01/14 06:53:14 jsg Exp $	*/
/* drm_linux_list.h -- linux list functions for the BSDs.
 * Created: Mon Apr 7 14:30:16 1999 by anholt@FreeBSD.org
 */
/*-
 * Copyright 2003 Eric Anholt
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <anholt@FreeBSD.org>
 *
 */

/*
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2016 Mellanox Technologies, Ltd.
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
#ifndef _DRM_LINUX_LIST_H_
#define _DRM_LINUX_LIST_H_

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/poison.h>
#include <linux/kernel.h>

#include <sys/queue.h>

#if 0
struct list_head {
	struct list_head *next;
	struct list_head *prev;
};
#endif

#define list_entry(ptr, type, member) container_of(ptr, type, member)

static inline void
INIT_LIST_HEAD(struct list_head *head) {
	(head)->next = head;
	(head)->prev = head;
}

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define DRM_LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline int
list_empty(const struct list_head *head) {
	return (head)->next == head;
}

static inline int
list_is_singular(const struct list_head *head) {
	return !list_empty(head) && ((head)->next == (head)->prev);
}

static inline int
list_is_first(const struct list_head *list,
    const struct list_head *head)
{
	return list->prev == head;
}

static inline int
list_is_last(const struct list_head *list,
    const struct list_head *head)
{
	return list->next == head;
}

static inline void
list_add(struct list_head *new, struct list_head *head) {
        (head)->next->prev = new;
        (new)->next = (head)->next;
        (new)->prev = head;
        (head)->next = new;
}

static inline void
list_add_tail(struct list_head *entry, struct list_head *head) {
	(entry)->prev = (head)->prev;
	(entry)->next = head;
	(head)->prev->next = entry;
	(head)->prev = entry;
}

#if 0
static inline void
_list_add(struct list_head *new, struct list_head *prev,
    struct list_head *next)
{

	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void
list_add(struct list_head *new, struct list_head *head)
{

	_list_add(new, head, head->next);
}

static inline void
list_add_tail(struct list_head *new, struct list_head *head)
{

	_list_add(new, head->prev, head);
}
#endif

static inline void
list_del(struct list_head *entry) {
	(entry)->next->prev = (entry)->prev;
	(entry)->prev->next = (entry)->next;
}

#if defined(__OpenBSD__)
#define __list_del_entry(x) list_del(x)
#else
static inline void
__list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	WRITE_ONCE(prev->next, next);
}

static inline void
__list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}
#endif

static inline void list_replace(struct list_head *old,
				struct list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void list_replace_init(struct list_head *old,
				     struct list_head *new)
{
	list_replace(old, new);
	INIT_LIST_HEAD(old);
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
	list_del(list);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *list,
    struct list_head *head)
{
	list_del(list);
	list_add_tail(list, head);
}

static inline void
list_rotate_to_front(struct list_head *list, struct list_head *head)
{
	list_del(head);
	list_add_tail(head, list);
}

static inline void
list_bulk_move_tail(struct list_head *head, struct list_head *first,
    struct list_head *last)
{
	first->prev->next = last->next;
	last->next->prev = first->prev;
	head->prev->next = first;
	first->prev = head->prev;
	last->next = head;
	head->prev = last;
}

static inline void
list_del_init(struct list_head *entry) {
	(entry)->next->prev = (entry)->prev;
	(entry)->prev->next = (entry)->next;
	INIT_LIST_HEAD(entry);
}

#define	prefetch(x)

 
static inline struct list_head *
list_first(const struct list_head *head)
{
	return head->next;
}

static inline struct list_head *
list_last(const struct list_head *head)
{
	return head->prev;
}

static inline int
list_empty_careful(const struct list_head *head)
{
	return (head == head->next) && (head == head->prev);
}

#define list_next_entry(pos, member)				\
	list_entry(((pos)->member.next), typeof(*(pos)), member)

#define list_prev_entry(pos, member)				\
	list_entry(((pos)->member.prev), typeof(*(pos)), member)

#define list_safe_reset_next(pos, n, member)			\
	n = list_next_entry(pos, member)

#define list_for_each(entry, head)				\
    for (entry = (head)->next; entry != head; entry = (entry)->next)

#define list_for_each_prev(entry, head) \
        for (entry = (head)->prev; entry != (head); \
                entry = entry->prev)

#define list_for_each_safe(entry, temp, head)			\
    for (entry = (head)->next, temp = (entry)->next;		\
	entry != head; 						\
	entry = temp, temp = entry->next)

#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_entry((head)->prev, __typeof(*pos), member),	\
	    n = list_entry((pos)->member.prev, __typeof(*pos), member);	\
	    &(pos)->member != (head);					\
	    pos = n, n = list_entry(n->member.prev, __typeof(*n), member))

#define list_for_each_entry_safe_from(pos, n, head, member) 		\
	for (n = list_entry(pos->member.next, __typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, __typeof(*n), member))

#define list_for_each_entry(pos, head, member)				\
    for (pos = list_entry((head)->next, __typeof(*pos), member);	\
	&pos->member != (head);					 	\
	pos = list_entry(pos->member.next, __typeof(*pos), member))

#define list_for_each_entry_from(pos, head, member)			\
    for (;								\
	&pos->member != (head);					 	\
	pos = list_entry(pos->member.next, __typeof(*pos), member))

#define list_for_each_entry_reverse(pos, head, member)			\
    for (pos = list_entry((head)->prev, __typeof(*pos), member);	\
	&pos->member != (head);					 	\
	pos = list_entry(pos->member.prev, __typeof(*pos), member))

#define list_for_each_entry_from_reverse(pos, head, member)		\
    for (;								\
	&pos->member != (head);					 	\
	pos = list_entry(pos->member.prev, __typeof(*pos), member))

#define list_for_each_entry_continue(pos, head, member)				\
    for (pos = list_entry((pos)->member.next, __typeof(*pos), member);	\
	&pos->member != (head);					 	\
	pos = list_entry(pos->member.next, __typeof(*pos), member))

#define list_for_each_entry_continue_reverse(pos, head, member)         \
        for (pos = list_entry(pos->member.prev, __typeof(*pos), member);  \
             &pos->member != (head);    				\
             pos = list_entry(pos->member.prev, __typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:        the type * to use as a loop cursor.
 * @n:          another type * to use as temporary storage
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, __typeof(*pos), member),	\
	    n = list_entry(pos->member.next, __typeof(*pos), member);	\
	    &pos->member != (head);					\
	    pos = n, n = list_entry(n->member.next, __typeof(*n), member))


#define list_first_entry(ptr, type, member) \
        list_entry((ptr)->next, type, member)

#define list_first_entry_or_null(ptr, type, member) \
	(list_empty(ptr) ? NULL : list_first_entry(ptr, type, member))

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

static inline void
_list_splice(const struct list_head *list, struct list_head *prev,  
    struct list_head *next)
{
	struct list_head *first;
	struct list_head *last;

	if (list_empty(list))
		return;
	first = list->next;
	last = list->prev;
	first->prev = prev;
	prev->next = first;
	last->next = next;
	next->prev = last;
}

static inline void
__list_splice(const struct list_head *list, struct list_head *prev,
    struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

static inline void
list_splice(const struct list_head *list, struct list_head *head)
{
	if (list_empty(list))
		return;

	__list_splice(list, head, head->next);
}

static inline void
list_splice_init(struct list_head *list, struct list_head *head)
{
	if (list_empty(list))
		return;

	__list_splice(list, head, head->next);
	INIT_LIST_HEAD(list);
}

static inline void
list_splice_tail(const struct list_head *list, struct list_head *head)
{
	if (list_empty(list))
		return;

	__list_splice(list, head->prev, head);
}

static inline void
list_splice_tail_init(struct list_head *list, struct list_head *head)
{
	if (list_empty(list))
		return;

	__list_splice(list, head->prev, head);
	INIT_LIST_HEAD(list);
}

#define LINUX_LIST_HEAD(name)	struct list_head name = { &(name), &(name) }

#if 0
struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};
#endif

#define hlist_entry(ptr, type, member) \
	((ptr) ? container_of(ptr, type, member) : NULL)

#if defined(__OpenBSD__)
static inline void
INIT_HLIST_HEAD(struct hlist_head *head) {
	head->first = NULL;
}
#else
#define	INIT_HLIST_HEAD(head) (head)->first = NULL
#endif

static inline int
hlist_empty(const struct hlist_head *head) {
	return head->first == NULL;
}

static inline void
hlist_add_head(struct hlist_node *new, struct hlist_head *head)
{
	new->next = head->first;
	if (head->first)
		head->first->pprev = &new->next;
	head->first = new;
	new->pprev = &head->first;
}

static inline int
hlist_unhashed(const struct hlist_node *node)
{
	return !node->pprev;
}

static inline void
hlist_del(struct hlist_node *node)
{
        if (node->next != NULL)
                node->next->pprev = node->pprev;
        *node->pprev = node->next;
}

#define	INIT_HLIST_NODE(node)						\
do {									\
	(node)->next = NULL;						\
	(node)->pprev = NULL;						\
} while (0)

#if defined(__OpenBSD__)
static inline void
hlist_del_init(struct hlist_node *node)
{
	if (node->next != NULL)
		node->next->prev = node->prev;
	*(node->prev) = node->next;
	node->next = NULL;
	node->prev = NULL;
}
#else
static inline void
hlist_del_init(struct hlist_node *node)
{
	if (hlist_unhashed(node))
		return;
	hlist_del(node);
	INIT_HLIST_NODE(node);
}
#endif

#define	hlist_for_each(pos, head)						\
	for (pos = (head)->first; pos; pos = pos->next)

#define hlist_entry_safe(pos, type, member) \
	(pos) ? hlist_entry(pos, type, member) : NULL

#if 0
#define hlist_for_each_entry(pos, head, member)				\
	for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member);\
	     pos;							\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))
#else
#define hlist_for_each_entry(pos, head, member)				\
	for (pos = hlist_entry((head)->first, __typeof(*pos), member);	\
	     pos != NULL;						\
	     pos = hlist_entry((pos)->member.next, __typeof(*pos), member))
#endif


#define	hlist_for_each_safe(p, n, head)					\
	for (p = (head)->first; p && ({ n = p->next; 1; }); p = n)


#define hlist_for_each_entry_continue(tp, p, field)			\
	for (p = (p)->next;						\
	    p ? (tp = hlist_entry(p, typeof(*tp), field)): NULL; p = p->next)

#define	hlist_for_each_entry_from(tp, p, field)				\
	for (; p ? (tp = hlist_entry(p, typeof(*tp), field)): NULL; p = p->next)

#define hlist_for_each_entry_safe(pos, n, head, member)			\
	for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member); \
	     (pos) && ({ n = (pos)->member.next; 1; });			\
	     pos = hlist_entry_safe(n, typeof(*(pos)), member))

#if 0 /* Not used anywhere */
#define	HLIST_HEAD_INIT { }
#define	HLIST_HEAD(name) struct hlist_head name = HLIST_HEAD_INIT
#endif

#if 0 /* Not used anywhere */
static inline void
hlist_add_before(struct hlist_node *n, struct hlist_node *next)
{

	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}
#endif
 
static inline void
hlist_add_after(struct hlist_node *n, struct hlist_node *next)
{

	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;
	if (next->next)
		next->next->pprev = &next->next;
}
 
#if 0 /* Not used anywhere */
static inline void
hlist_move_list(struct hlist_head *old, struct hlist_head *new)
{

	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}
#endif

#if 0 /* Now in tools/tools/drm/radeon/mregtable/mkregtable.c */
static inline void __list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	if (list_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}
#endif

void drm_list_sort(void *priv, struct list_head *head, int (*cmp)(void *priv,
    struct list_head *a, struct list_head *b));

#define hlist_add_head_rcu(n, h)	hlist_add_head(n, h)

#define hlist_del_init_rcu(n)		hlist_del_init(n)

#endif /* _DRM_LINUX_LIST_H_ */

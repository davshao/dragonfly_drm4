/* Public domain. */

/*
 * Copyright (c) 2019-2020 Jonathan Gray <jsg@openbsd.org>
 * Copyright (c) 2020 François Tigeot <ftigeot@wolfpond.org>
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _LINUX_LLIST_H
#define _LINUX_LLIST_H

#if defined(__OpenBSD__)
#include <sys/atomic.h>
#else
#include <machine/atomic.h>
#endif
// #include <linux/atomic.h>
// #include <linux/kernel.h>

struct llist_node {
	struct llist_node *next;
};

struct llist_head {
	struct llist_node *first;
};

#define llist_entry(ptr, type, member) \
	((ptr) ? container_of(ptr, type, member) : NULL)

static inline struct llist_node *
llist_del_all(struct llist_head *head)
{
#if defined(__OpenBSD__)
	return atomic_swap_ptr(&head->first, NULL);
#else
	return atomic_swap_ptr((void *)&head->first, NULL);
#endif
}

static inline struct llist_node *
llist_del_first(struct llist_head *head)
{
	struct llist_node *first, *next;

	do {
		first = head->first;
		if (first == NULL)
			return NULL;
		next = first->next;
	} while (atomic_cas_ptr(&head->first, first, next) != first);

	return first;
}

static inline bool
llist_add(struct llist_node *new, struct llist_head *head)
{
#if defined(__OpenBSD__)
	struct llist_node *first;

	do {
		new->next = first = head->first;
	} while (atomic_cas_ptr(&head->first, first, new) != first);

	return (first == NULL);
#else /* previous DragonFly */
	struct llist_node *first = READ_ONCE(head->first);

	do {
		new->next = first;
	} while (cmpxchg(&head->first, first, new) != first);
#endif

	return (first == NULL);
}

static inline bool
llist_add_batch(struct llist_node *new_first, struct llist_node *new_last,
    struct llist_head *head)
{
	struct llist_node *first;

	do {
		new_last->next = first = head->first;
	} while (atomic_cas_ptr(&head->first, first, new_first) != first);

	return (first == NULL);
}

static inline void
init_llist_head(struct llist_head *head)
{
	head->first = NULL;
}

static inline bool
llist_empty(struct llist_head *head)
{
	return (head->first == NULL);
}

#define llist_for_each_safe(pos, n, node)				\
	for ((pos) = (node);						\
	    (pos) != NULL &&						\
	    ((n) = (pos)->next, pos);					\
	    (pos) = (n))

#if defined(__OpenBSD__)
#define llist_for_each_entry_safe(pos, n, node, member) 		\
	for (pos = llist_entry((node), __typeof(*pos), member); 	\
	    ((char *)(pos) + offsetof(typeof(*(pos)), member)) != NULL && \
	    (n = llist_entry(pos->member.next, __typeof(*pos), member), pos); \
	    pos = n)
#else /* previous DragonFly */
#define llist_for_each_entry_safe(pos, n, node, member) 		\
	for (pos = llist_entry((node), __typeof(*pos), member); 	\
	    pos != NULL &&						\
	    (n = llist_entry(pos->member.next, __typeof(*pos), member), pos); \
	    pos = n)
#endif

#if defined(__OpenBSD__)
#define llist_for_each_entry(pos, node, member)				\
	for ((pos) = llist_entry((node), __typeof(*(pos)), member);	\
	    ((char *)(pos) + offsetof(typeof(*(pos)), member)) != NULL;	\
	    (pos) = llist_entry((pos)->member.next, __typeof(*(pos)), member))
#else /* previous DragonFly */
#define llist_for_each_entry(pos, node, member)				\
	for ((pos) = llist_entry((node), __typeof(*(pos)), member);	\
	    (pos) != NULL;						\
	    (pos) = llist_entry((pos)->member.next, __typeof(*(pos)), member))
#endif

#endif

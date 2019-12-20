/* SPDX-License-Identifier: MIT */
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: 2012-2016, Sven Eckelmann <sven@narfation.org>
 */

#ifndef __RBTREE_COMMON_PRIOQUEUE_H__
#define __RBTREE_COMMON_PRIOQUEUE_H__

#include <stddef.h>

#include "../rbtree.h"
#include "common.h"

struct rb_prioqueue {
	struct rb_root root;
	struct rb_node *min_node;
};

static __inline__ void rb_prioqueue_init(struct rb_prioqueue *queue)
{
	INIT_RB_ROOT(&queue->root);
	queue->min_node = NULL;
}

static __inline__ void
rb_prioqueue_insert_unbalanced(struct rb_prioqueue *queue,
			       struct rbitem *new_entry)
{
	struct rb_node *parent = NULL;
	struct rb_node **cur_nodep = &queue->root.node;
	struct rbitem *cur_entry;
	int isminimal = 1;

	while (*cur_nodep) {
		cur_entry = rb_entry(*cur_nodep, struct rbitem, rb);

		parent = *cur_nodep;
		if (cmpint(&new_entry->i, &cur_entry->i) <= 0) {
			cur_nodep = &((*cur_nodep)->left);
		} else {
			cur_nodep = &((*cur_nodep)->right);
			isminimal = 0;
		}
	}

	if (isminimal)
		queue->min_node = &new_entry->rb;

	rb_link_node(&new_entry->rb, parent, cur_nodep);
}

static __inline__ struct rbitem *
rb_prioqueue_pop_unbalanced(struct rb_prioqueue *queue)
{
	struct rbitem *item;

	if (!queue->min_node)
		return NULL;

	item = rb_entry(queue->min_node, struct rbitem, rb);
	queue->min_node = rb_next(queue->min_node);

	rb_erase_node(&item->rb, &queue->root);

	return item;
}

static __inline__ void
rb_prioqueue_insert_balanced(struct rb_prioqueue *queue,
			       struct rbitem *new_entry)
{
	struct rb_node *parent = NULL;
	struct rb_node **cur_nodep = &queue->root.node;
	struct rbitem *cur_entry;
	int isminimal = 1;

	while (*cur_nodep) {
		cur_entry = rb_entry(*cur_nodep, struct rbitem, rb);

		parent = *cur_nodep;
		if (cmpint(&new_entry->i, &cur_entry->i) <= 0) {
			cur_nodep = &((*cur_nodep)->left);
		} else {
			cur_nodep = &((*cur_nodep)->right);
			isminimal = 0;
		}
	}

	if (isminimal)
		queue->min_node = &new_entry->rb;

	rb_insert(&new_entry->rb, parent, cur_nodep, &queue->root);
}

static __inline__ struct rbitem *
rb_prioqueue_pop_balanced(struct rb_prioqueue *queue)
{
	struct rbitem *item;

	if (!queue->min_node)
		return NULL;

	item = rb_entry(queue->min_node, struct rbitem, rb);
	queue->min_node = rb_next(queue->min_node);

	rb_erase(&item->rb, &queue->root);

	return item;
}

#endif /* __RBTREE_COMMON_PRIOQUEUE_H__ */

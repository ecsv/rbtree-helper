/* SPDX-License-Identifier: MIT */
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: 2012-2016, Sven Eckelmann <sven@narfation.org>
 */

#ifndef __RBTREE_COMMON_TREEOPS_H__
#define __RBTREE_COMMON_TREEOPS_H__

#include <stddef.h>
#include <stdint.h>

#include "../rbtree.h"
#include "common.h"

static __inline__ void rbitem_insert(struct rb_root *root,
				     struct rbitem *new_entry)
{
	struct rb_node *parent = NULL;
	struct rb_node **cur_nodep = &root->node;
	struct rbitem *cur_entry;

	while (*cur_nodep) {
		cur_entry = rb_entry(*cur_nodep, struct rbitem, rb);

		parent = *cur_nodep;
		if (cmpint(&new_entry->i, &cur_entry->i) <= 0)
			cur_nodep = &((*cur_nodep)->left);
		else
			cur_nodep = &((*cur_nodep)->right);
	}

	rb_insert(&new_entry->rb, parent, cur_nodep, root);
}

static __inline__ struct rbitem *rbitem_find(struct rb_root *root, uint16_t x)
{
	struct rb_node **cur_nodep = &root->node;
	struct rbitem *cur_entry;
	int res;

	while (*cur_nodep) {
		cur_entry = rb_entry(*cur_nodep, struct rbitem, rb);

		res = cmpint(&x, &cur_entry->i);
		if (res == 0)
			return cur_entry;

		if (res < 0)
			cur_nodep = &((*cur_nodep)->left);
		else
			cur_nodep = &((*cur_nodep)->right);
	}

	return NULL;
}

#endif /* __RBTREE_COMMON_TREEOPS_H__ */

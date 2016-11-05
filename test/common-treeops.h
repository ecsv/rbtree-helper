/* Minimal red-black-tree helper functions test
 *
 * Copyright (c) 2012-2016, Sven Eckelmann <sven@narfation.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __RBTREE_COMMON_TREEOPS_H__
#define __RBTREE_COMMON_TREEOPS_H__

#include <stddef.h>
#include <stdint.h>

#include "../rbtree.h"
#include "common.h"

static __inline__ void rbitem_insert_unbalanced(struct rb_root *root,
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

	rb_link_node(&new_entry->rb, parent, cur_nodep);
}

static __inline__ void rbitem_insert_balanced(struct rb_root *root,
					      struct rbitem *new_entry)
{
	rbitem_insert_unbalanced(root, new_entry);
	rb_insert_color(&new_entry->rb, root);
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

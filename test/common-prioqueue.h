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
rb_prioqueue_insert(struct rb_prioqueue *queue, struct rbitem *new_entry)
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

static __inline__ struct rbitem *rb_prioqueue_pop(struct rb_prioqueue *queue)
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

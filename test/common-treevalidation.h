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

#ifndef __RBTREE_COMMON_TREEVALIDATION_H__
#define __RBTREE_COMMON_TREEVALIDATION_H__

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "../rbtree.h"
#include "common.h"

struct min_max_depth {
	size_t min;
	size_t max;
	size_t black_min;
	size_t black_max;
};

static __inline__ void check_node_order(struct rb_node *node,
					struct rb_node *parent,
					const uint8_t *skiplist, uint16_t *pos,
					uint16_t size)
{
	struct rbitem *item;

	if (!node)
		return;

	assert(rb_parent(node) == parent);

	check_node_order(node->left, node, skiplist, pos, size);

	while (*pos < size && skiplist[*pos])
		(*pos)++;
	assert(*pos < size);

	item = rb_entry(node, struct rbitem, rb);
	assert(item->i == *pos);
	(*pos)++;

	check_node_order(node->right, node, skiplist, pos, size);
}

static __inline__ void check_root_order(const struct rb_root *root,
					const uint8_t *skiplist, uint16_t size)
{
	uint16_t pos = 0;

	check_node_order(root->node, NULL, skiplist, &pos, size);

	while (pos < size && skiplist[pos])
		pos++;

	assert(size == pos);
}

static __inline__ struct min_max_depth
get_min_max_node(const struct rb_node *node)
{
	struct min_max_depth depth = {1, 1, 1, 1};
	struct min_max_depth depth_left, depth_right;
	size_t blackcnt;

	if (!node)
		return depth;

	depth_left = get_min_max_node(node->left);
	depth_right = get_min_max_node(node->right);

	/* count all nodes */
	assert(depth_left.min * 2 >= depth_left.max);
	assert(depth_right.min * 2 >= depth_right.max);

	if (depth_left.min < depth_right.min)
		depth.min = depth_left.min + 1;
	else
		depth.min = depth_right.min + 1;

	if (depth_left.max > depth_right.max)
		depth.max = depth_left.max + 1;
	else
		depth.max = depth_right.max + 1;

	/* only count black nodes */
	assert(depth_left.black_min == depth_left.black_max);
	assert(depth_right.black_min == depth_right.black_max);

	if (rb_color(node) == RB_BLACK)
		blackcnt = 1;
	else
		blackcnt = 0;

	if (depth_left.black_min < depth_right.black_min)
		depth.black_min = depth_left.black_min + blackcnt;
	else
		depth.black_min = depth_right.black_min + blackcnt;

	if (depth_left.black_max > depth_right.black_max)
		depth.black_max = depth_left.black_max + blackcnt;
	else
		depth.black_max = depth_right.black_max + blackcnt;

	return depth;
}

static __inline__ struct min_max_depth
get_min_max_root(const struct rb_root *root)
{
	return get_min_max_node(root->node);
}

static __inline__ void check_depth(const struct rb_root *root)
{
	struct min_max_depth depths;

	depths = get_min_max_root(root);
	assert(depths.min * 2 >= depths.max);
	assert(depths.black_min == depths.black_max);
}

static __inline__ void check_rb_node(const struct rb_node *node)
{
	if (!node)
		return;

	/* no two consecutive red */
	if (rb_color(node) == RB_RED) {
		assert(!node->left || rb_color(node->left) == RB_BLACK);
		assert(!node->right || rb_color(node->right) == RB_BLACK);
	}

	check_rb_node(node->left);
	check_rb_node(node->right);
}

static __inline__ void check_rb_nodes(const struct rb_root *root)
{
	if (root->node)
		assert(rb_color(root->node) == RB_BLACK);

	check_rb_node(root->node);
}

#endif /* __RBTREE_COMMON_TREEVALIDATION_H__ */

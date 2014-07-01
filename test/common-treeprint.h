/* Minimal red-black-tree helper functions test
 *
 * Copyright (c) 2012-2014, Sven Eckelmann <sven@narfation.org>
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

#ifndef __RBTREE_COMMON_TREEPRINT_H__
#define __RBTREE_COMMON_TREEPRINT_H__

#include "../rbtree.h"
#include "common.h"
#include <stdio.h>

static __inline__ void printnode(const struct rb_node *node, size_t depth,
				 char prefix)
{
	const struct rbitem *item;
	size_t i;

	if (!node) {
		for (i = 0; i < depth; i++)
			printf("     ");

		printf("%cB-\n", prefix);
		return;
	}

	item = rb_entry(node, struct rbitem, rb);

	printnode(node->right, depth+1, '/');

	for (i = 0; i < depth; i++)
		printf("     ");

	if (rb_color(node) == RB_RED)
		printf("%cr", prefix);
	else
		printf("%cB", prefix);

	printf("%03u\n", item->i);

	printnode(node->left, depth+1, '\\');

}

static __inline__ void printtree(const struct rb_root *root)
{
	printnode(root->node, 0, '*');
}

static __inline__ void printnode_dot(const struct rb_node *node,
				     size_t *nilcnt)
{
	const struct rbitem *item;
	const struct rbitem *citem;
	const char *color;

	if (!node)
		return;

	item = rb_entry(node, struct rbitem, rb);
	if (rb_color(node) == RB_RED)
		color = "red";
	else
		color = "black";
	printf("%03u [color=\"%s\"];\n", item->i, color);

	if (node->left) {
		citem = rb_entry(node->left, struct rbitem, rb);
		if (rb_color(node->left) == RB_RED)
			color = "red";
		else
			color = "black";

		printf("%03u:sw -> %03u [color=\"%s\"];\n", item->i, citem->i,
		       color);
	} else {
		printf("nil%zu [label=\"NIL\", shape=box, color=\"black\"];\n",
		       *nilcnt);
		printf("%03u:sw -> nil%zu [color=\"black\"];\n", item->i,
		       *nilcnt);

		(*nilcnt)++;
	}

	if (node->right) {
		citem = rb_entry(node->right, struct rbitem, rb);
		if (rb_color(node->right) == RB_RED)
			color = "red";
		else
			color = "black";

		printf("%03u:se -> %03u [color=\"%s\"];\n", item->i, citem->i,
			color);
	} else {
		printf("nil%zu [label=\"NIL\", shape=box, color=\"black\"];\n",
		       *nilcnt);
		printf("%03u:se -> nil%zu [color=\"black\"];\n", item->i,
		       *nilcnt);

		(*nilcnt)++;
	}

	printnode_dot(node->left, nilcnt);
	printnode_dot(node->right, nilcnt);
}

static __inline__ void printtree_dot(const struct rb_root *root)
{
	size_t nilcnt = 0;

	printf("digraph G {\n");
	printf("  graph [ordering=\"out\"];\n");

	printnode_dot(root->node, &nilcnt);

	printf("}\n");
}

#endif /* __RBTREE_COMMON_TREEPRINT_H__ */

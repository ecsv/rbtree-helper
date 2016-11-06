/* Minimal red-black-tree helper functions
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

#include "rbtree.h"

#include <stddef.h>

/**
 * rb_set_parent() - Set parent of node
 * @node: pointer to the rb node
 * @parent: pointer to the new parent node
 */
static void rb_set_parent(struct rb_node *node, struct rb_node *parent)
{
#ifndef RB_PARENT_COLOR_COMBINATION
	node->parent = parent;
#else
	node->parent_color = (unsigned long)parent | (node->parent_color & 1lu);
#endif
}

/**
 * rb_set_color() - Set color of node
 * @node: pointer to the rb node
 * @color: new color of the node
 */
static void rb_set_color(struct rb_node *node, enum rb_node_color color)
{
#ifndef RB_PARENT_COLOR_COMBINATION
	node->color = color;
#else
	node->parent_color = (node->parent_color & ~1lu) | color;
#endif
}

/**
 * rb_change_child() - Fix child entry of parent node
 * @old_node: rb node to replace
 * @new_node: rb node replacing @old_node
 * @parent: parent of @old_node
 * @root: pointer to rb root
 *
 * Detects if @old_node is left/right child of @parent or if it gets inserted
 * as as new root. These entries are then updated to point to @new_node.
 *
 * @old_node and @root must not be NULL.
 */
static void rb_change_child(struct rb_node *old_node, struct rb_node *new_node,
			    struct rb_node *parent, struct rb_root *root)
{
	if (parent) {
		if (parent->left == old_node)
			parent->left = new_node;
		else
			parent->right = new_node;
	} else {
		root->node = new_node;
	}
}

/**
 * rb_rotate_switch_parents() - set parent for switched nodes after rotate
 * @node_top: rb node which became the new top node
 * @node_child: rb node which became the new child node
 * @node_child2: ex'child of @node_top which now is now 2. child of @node_child
 * @root: pointer to rb root
 * @color: new color for @node_child (most of the time old color of node_top)
 *
 * @node_top must have been a valid child of @node_child. The child changes
 * for the rotation in @node_child and @node_top must already be finished.
 * The switch of parents for @node_top, @node_child and @node_child2
 * (when it exists) is peformend. The change of the child entry of the new
 * parent of @node_top is done afterwards.
 */
static void rb_rotate_switch_parents(struct rb_node *node_top,
				     struct rb_node *node_child,
				     struct rb_node *node_child2,
				     struct rb_root *root,
				     enum rb_node_color color)
{
	/* switch parents and set new color */
	rb_set_parent_color(node_top, rb_parent(node_child),
			    rb_color(node_child));
	rb_set_parent_color(node_child, node_top, color);

	/* switch parent of child2 from child to top */
	if (node_child2)
		rb_set_parent(node_child2, node_child);

	/* parent of node_top must get its child pointer get fixed */
	rb_change_child(node_child, node_top, rb_parent(node_top), root);
}

/**
 * rb_insert_color() - Go tree upwards and rebalance it after insert
 * @node: pointer to the new node
 * @root: pointer to rb root
 *
 * The tree is traversed from bottom to the top starting at @node. right leaning
 * 3-nodes are rotated left, unbalanced 4-nodes are rotated to the left and
 * balanced 4-nodes are splitted again into to 2-nodes (color flip).
 *
 * When the tree was a LLRB before the link of the new node then the resulting
 * tree will again be a LLRB tree
 */
void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *parent;
	struct rb_node *tmp;

	/* go tree upwards and fix the nodes on the way */
	while (node) {
		parent = rb_parent(node);

		/* rotate 3-node to left when right child is red */
		if (rb_is_red(node->right)) {
			/* rotate */
			tmp = node->right;
			node->right = tmp->left;
			tmp->left = node;

			/* fix colors and parent entries
			 * node must become red during rotate
			 */
			rb_rotate_switch_parents(tmp, node, node->right, root,
						 RB_RED);

			node = tmp;
		}

		/* rotate right when two consecutive left nodes are red */
		if (rb_is_red(node->left) &&
		    rb_is_red(node->left->left)) {
			/* rotate */
			tmp = node->left;
			node->left = tmp->right;
			tmp->right = node;

			/* fix colors and parent entries
			 * node must become red during rotate
			 */
			rb_rotate_switch_parents(tmp, node, node->left, root,
						 RB_RED);

			node = tmp;
		}

		/* flip color/split 4-node into 2-nodes */
		if (rb_is_red(node->left) &&
		    rb_is_red(node->right)) {
			rb_set_color(node, RB_RED);
			rb_set_color(node->left, RB_BLACK);
			rb_set_color(node->right, RB_BLACK);
		}

		/* stop when no more fixes required on red path */
		if (rb_color(node) == RB_BLACK)
			break;

		/* reached red root, mark it black */
		if (!parent)
			rb_set_parent_color(node, NULL, RB_BLACK);

		node = parent;
	}
}

/**
 * rb_erase_node() - Remove rb node from tree
 * @node: pointer to the node
 * @root: pointer to rb root
 *
 * The node is only removed from the tree. Neither the memory of the removed
 * node nor the memory of the entry containing the node is free'd. The node
 * has to be handled like an uninitialized node. Accessing the parent or
 * right/left pointer of the node is not safe.
 *
 * WARNING The removed node may cause the tree to be become unbalanced or
 * violate any rules of the red black tree. A call to rb_erase_color after
 * rb_erase_node is therefore always required to rebalance the tree correctly.
 * rb_erase can be used as helper to run both steps at the same time.
 *
 * Return: node which is double black and has to be rebalanced, NULL if no
 *  rebalance is necessary
 */
struct rb_node *rb_erase_node(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *smallest;
	struct rb_node *smallest_parent;
	struct rb_node *dblack;
	enum rb_node_color smallest_color;

	/* no child */
	if (!node->left && !node->right) {
		rb_change_child(node, NULL, rb_parent(node), root);

		/* a red node can be ignored because the parent is a 3 node
		 * which gets then converted to 2 node after delete. If it is
		 * black then the parent node might get double black and thus
		 * has to be rebalanced
		 */
		if (rb_is_red(node))
			return NULL;
		else
			return rb_parent(node);
	}

	/* one child, left */
	if (node->left && !node->right) {
		rb_set_parent_color(node->left, rb_parent(node), RB_BLACK);
		rb_change_child(node, node->left, rb_parent(node), root);

		/* the left child must be red when there is no right child
		 * (3-node). Otherwise the subtrees would have different
		 * heights. So the child gets black and the red link is dropped.
		 * Nothing to rebalance anymore
		 */
		return NULL;
	}

	/* one child, right
	 * node->right not tested due to previous two cases
	 */
	if (!node->left) {
		rb_set_parent_color(node->right, rb_parent(node), RB_BLACK);
		rb_change_child(node, node->right, rb_parent(node), root);

		/* the right child must be red when there is no left child
		 * (3-node). Otherwise the subtrees would have different
		 * heights. But this is a LLRB and thus a right red child
		 * never happens after a rebalance. Still leaving this when
		 * function is (mis)used for non-LLRB
		 */
		return NULL;
	}

	/* two children, take smallest of right (grand)children */
	smallest = node->right;
	while (smallest->left)
		smallest = smallest->left;

	smallest_parent = rb_parent(smallest);
	smallest_color = rb_color(smallest);
	if (smallest == node->right)
		dblack = node->right;
	else
		dblack = smallest_parent;

	/* move right child of smallest one up
	 * But this is a LLRB and thus a right child with no left child never
	 * happens after a rebalance. Still leaving this when function is
	 * (mis)used for non-LLRB
	 */
	if (smallest->right)
		rb_set_parent(smallest->right, smallest_parent);
	rb_change_child(smallest, smallest->right, smallest_parent, root);

	/* exchange node with smallest */
	rb_set_parent_color(smallest, rb_parent(node), rb_color(node));

	smallest->left = node->left;
	rb_set_parent(smallest->left, smallest);

	smallest->right = node->right;
	if (smallest->right)
		rb_set_parent(smallest->right, smallest);

	rb_change_child(node, smallest, rb_parent(node), root);

	/* a red node can be ignored because the parent is a 3 node
	 * which gets then converted to 2 node after smallest node is moved up.
	 * If it is black then the parent node might get double black and thus
	 * has to be rebalanced
	 */
	if (smallest_color == RB_RED)
		return NULL;
	else
		return dblack;
}

/**
 * rb_erase_color() - Go tree upwards and rebalance it after erase_node
 * @parent: double black node which has to be rebalanced after child was removed
 * @root: pointer to rb root
 *
 * The tree is traversed from bottom to the top starting at @parent. The
 * rebalancing is done via restructuring, recoloring and adjustment (requires
 * right-leaning restructuring/recoloring)
 *
 * When the tree was a LLRB before the erase of the node then the resulting
 * tree will again be a LLRB tree
 */
void rb_erase_color(struct rb_node *parent, struct rb_root *root)
{
	struct rb_node *gparent;
	struct rb_node *sibling;
	struct rb_node *tmp;
	int coming_from_right = 0;

	/* the right child was removed when it is missing
	 * otherwise the left child was the smallest during erase and thus
	 * it was removed
	 */
	if (!parent->right)
		coming_from_right = 1;

	/* go tree upwards and fix the nodes on the way */
	while (1) {
		gparent = rb_parent(parent);

		/* if left child of right sibling is red
		 * rotate sibling and parent to convert unbalanced 2x 2-nodes +
		 * 1x 3-node to balanced 3x 2-nodes
		 *
		 * red node from sibling is borrowed to eliminate double black
		 *
		 * LLRB-safe "restructuring"
		 */
		if (!coming_from_right && rb_is_red(parent->right->left)) {

			/* rotate sibling's tree to right
			 * red becomes right child of new sibling
			 */
			sibling = parent->right;
			tmp = sibling->left;
			sibling->left = tmp->right;
			tmp->right = sibling;

			/* fix colors and parent entries for sibling tree
			 * sibling must become red
			 */
			rb_rotate_switch_parents(tmp, sibling, sibling->left,
						 root, RB_RED);

			/* rotate parents tree to the left
			 * parent becomes red and sibling, red can be borrowed
			 * (changed to black) for double black of node
			 */
			tmp = parent->right;
			parent->right = tmp->left;
			tmp->left = parent;

			/* fix colors and parent entries for parent tree
			 * parent must have become black
			 */
			rb_rotate_switch_parents(tmp, parent, parent->right,
						 root, RB_BLACK);

			/**
			 * the rotation in the parent tree (both child black)
			 * towards the node increased the black-height
			 * (below tmp/previously parent) of the right tree+1 and
			 * of the left tree-1. The double black node can
			 * therefore be dropped. The red right child below tmp
			 * has to be changed to have the right tree again at the
			 * same height
			 */
			rb_set_color(tmp->right, RB_BLACK);

			break;
		}

		/* if left child of left sibling is red
		 * rotate sibling and parent to convert unbalanced 1x 2-nodes +
		 * 1x 3-node to balanced 2x 2-nodes
		 *
		 * red node from sibling is borrowed to eliminate double black
		 *
		 * LLRB-safe "restructuring"
		 */
		if (coming_from_right && rb_color(parent->left) == RB_BLACK &&
		    rb_is_red(parent->left->left)) {

			/* rotate parents tree to the right */
			tmp = parent->left;
			parent->left = tmp->right;
			tmp->right = parent;

			/* fix colors and parent entries for parent tree
			 * parent must have become black
			 */
			rb_rotate_switch_parents(tmp, parent, parent->left,
						 root, RB_BLACK);

			/**
			 * the rotation increased the black-height of the right
			 * tree (below tmp/previously parent) + 1. The left
			 * tree has to compensate by turning the red node to
			 * black (splitting 3-node into 2x 2-nodes)
			 */
			rb_set_color(tmp->left, RB_BLACK);

			break;
               }

               /* left sibling is red
		* rotate 3-node towards right and split it into 2x 2-nodes
		*
		* LLRB-safe "adjustment"
		*/
               if (coming_from_right && rb_is_red(parent->left)) {
			/* rotate right */
			tmp = parent->left;
			parent->left = tmp->right;
			tmp->right = parent;

			/* fix colors and parent entries
			 *
			 * the rotation of the three node kept the black-height
			 * balance the same. Parent becomes red.
			 *
			 * this caused a right-leaning red node
			 */
			rb_rotate_switch_parents(tmp, parent, parent->left,
						 root, RB_RED);

			/* start extra recoloring or restructure + rotate
			 * to fix the height and the right-leaning red node
			 */
			gparent = tmp;

			/* recolor when sibling's children are black
			 * already known that parent (right leaning is red
			 * and right sibling is black
			 *
			 * recoloring fixes the right leaning red parent
			 */
			if (!rb_is_red(parent->left->left)) {
				rb_set_color(parent, RB_BLACK);
				rb_set_color(parent->left, RB_RED);
				break;
			}

			/* restructure when siblings left child is red
			 * rotation has to be done later to fix new grandparent
			 *
			 * rotate parents tree to the right first
			 */
			tmp = parent->left;
			parent->left = tmp->right;
			tmp->right = parent;

			/* fix colors and parent entries for parent tree
			 * parent must have become black
			 */
			rb_rotate_switch_parents(tmp, parent, parent->left,
						 root, RB_BLACK);

			/**
			 * the rotation increased the black-height of
			 * the right tree (below tmp/previously parent)
			 * + 1. The left tree has to compensate by
			 * turning the red node to black (splitting
			 * 3-node into 2x 2-nodes)
			 */
			rb_set_color(tmp->left, RB_BLACK);

			/* rotate gparent to fix right-leaning red */
			tmp = gparent->right;
			gparent->right = tmp->left;
			tmp->left = gparent;

			/* fix colors and parent entries
			 * gparent must become red during rotate
			 */
			rb_rotate_switch_parents(tmp, gparent, gparent->right,
						 root, RB_RED);
			break;
		}

		/* left sibling's children are black and parent is red
		 * remove parent from 3-node, create new left leaning 3-node
		 * with sibbling
		 *
		 * LLRB-safe "recoloring"
		 */
		if (rb_is_red(parent) && coming_from_right &&
		    parent->left && !rb_is_red(parent->left->left)) {
			/* increase black-height of parent */
			rb_set_color(parent, RB_BLACK);

			/* decrease black-height of sibbling  */
			rb_set_color(parent->left, RB_RED);

			break;
		}

		/* right sibling's children are black and parent is red
		 * remove parent from 3-node, create new left leaning 3-node
		 * with sibbling
		 *
		 * LLRB-safe "recoloring"
		 */
		if (rb_is_red(parent) && !coming_from_right &&
		    parent->right && !rb_is_red(parent->right->left)) {
			/* increase black-height of parent */
			rb_set_color(parent, RB_BLACK);

			/* decrease black-height of sibbling  */
			rb_set_color(parent->right, RB_RED);

			/* rotate left to make LLRB */
			tmp = parent->right;
			parent->right = tmp->left;
			tmp->left = parent;

			/* fix colors and parent entries
			 * node must become red during rotate
			 */
			rb_rotate_switch_parents(tmp, parent, parent->right,
						 root, RB_RED);

			break;
		}

		/* left sibling is black and parent is black
		 * (new) parent becomes double black -> continue upwards
		 *
		 * LLRB-safe "recoloring"
		 */
		if (!rb_is_red(parent) && coming_from_right &&
		    !rb_is_red(parent->left) && !rb_is_red(parent->right) &&
		    parent->left && !rb_is_red(parent->left->left)) {
			/* decrease black-height of sibbling  */
			rb_set_color(parent->left, RB_RED);

			/* continue at grand-parent to fix parents
			 * 'double-black'ness
			 */
		}

		/* right sibling is black and parent is black
		 * (new) parent becomes double black -> continue upwards
		 *
		 * LLRB-safe "recoloring"
		 */
		if (!rb_is_red(parent) && !coming_from_right &&
		    !rb_is_red(parent->left) &&
		    parent->right && !rb_is_red(parent->right->left)) {
			/* decrease black-height of sibbling  */
			rb_set_color(parent->right, RB_RED);

			/* rotate left to make LLRB again */
			tmp = parent->right;
			parent->right = tmp->left;
			tmp->left = parent;

			/* fix colors and parent entries
			 * node must become red during rotate
			 */
			rb_rotate_switch_parents(tmp, parent, parent->right,
						 root, RB_RED);

			/* continue at grand-parent to fix parents
			 * 'double-black'ness
			 */
			parent = tmp;
			gparent = rb_parent(tmp);
		}

		/* reached root, mark it black */
		if (!gparent) {
			rb_set_parent_color(parent, NULL, RB_BLACK);
			break;
		}

		if (gparent->left == parent)
			coming_from_right = 0;
		else
			coming_from_right = 1;

		parent = gparent;
	}
}

/**
 * rb_first() - Find leftmost rb node in tree
 * @root: pointer to rb root
 *
 * Return: pointer to leftmost node. NULL when @root is empty.
 */
struct rb_node *rb_first(const struct rb_root *root)
{
	struct rb_node *node = root->node;

	if (!node)
		return node;

	/* descend down via smaller/preceding child */
	while (node->left)
		node = node->left;

	return node;
}

/**
 * rb_last() - Find rightmost rb node in tree
 * @root: pointer to rb root
 *
 * Return: pointer to rightmost node. NULL when @root is empty.
 */
struct rb_node *rb_last(const struct rb_root *root)
{
	struct rb_node *node = root->node;

	if (!node)
		return node;

	/* descend down via larger/succeeding child */
	while (node->right)
		node = node->right;

	return node;
}

/**
 * rb_next() - Find successor node in tree
 * @node: starting rb node for search
 *
 * Return: pointer to successor node. NULL when no successor of @node exist.
 */
struct rb_node *rb_next(struct rb_node *node)
{
	struct rb_node *parent;

	/* there is a right child - next node must be the leftmost under it */
	if (node->right) {
		node = node->right;
		while (node->left)
			node = node->left;

		return node;
	}

	/* otherwise check if we have a parent (and thus maybe siblings) */
	parent = rb_parent(node);
	if (!parent)
		return parent;

	/* go up the tree until the path connecting both is the left child
	 * pointer and therefore the parent is the next node
	 */
	while (parent && parent->right == node) {
		node = parent;
		parent = rb_parent(node);
	}

	return parent;
}

/**
 * rb_prev() - Find predecessor node in tree
 * @node: starting rb node for search
 *
 * Return: pointer to predecessor node. NULL when no predecessor of @node exist.
 */
struct rb_node *rb_prev(struct rb_node *node)
{
	struct rb_node *parent;

	/* there is a left child - prev node must be the rightmost under it */
	if (node->left) {
		node = node->left;
		while (node->right)
			node = node->right;

		return node;
	}

	/* otherwise check if we have a parent (and thus maybe siblings) */
	parent = rb_parent(node);
	if (!parent)
		return parent;

	/* go up the tree until the path connecting both is the right child
	 * pointer and therefore the parent is the prev node
	 */
	while (parent && parent->left == node) {
		node = parent;
		parent = rb_parent(node);
	}

	return parent;
}

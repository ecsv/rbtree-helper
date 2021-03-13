// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include "rbtree.h"

#include <stdbool.h>
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
 * rb_insert_recolor() - Rebalance right subtree via recolor
 * @gparent: black grandparent of unbalanced subtree
 * @root: pointer to rb root
 *
 * The grandparent has two red children and one red grandchildren. This
 * is a violation of the red black tree requirements (two consecutive red
 * nodes). This can be moved from the grandchildren up to the grandparent by
 * recoloring the children of the grandparent from red to black.
 *
 * But this also means that the grandparent must now be (temporarily) recolored
 * red and the problem of the potentially two consecutive red nodes are moved
 * up the tree. The rebalancing code must therefore continue the process at
 * the grandparent.
 */
static void rb_insert_recolor(struct rb_node *gparent)
{
	/* switch red parent of our red node and the red uncle to black */
	rb_set_color(gparent->left, RB_BLACK);
	rb_set_color(gparent->right, RB_BLACK);

	/* move the red color of uncle/parent up to previously black grand
	 * parent
	 */
	rb_set_color(gparent, RB_RED);
}

/**
 * rb_insert_rotate_left() - Rotate at @node to the left
 * @node: pointer to the new node
 * @root: pointer to rb root
 *
 * The right child of @node is moved up the tree and the @node becomes the
 * left child of this tree. The left node is assumed to be black (or non
 * existing). The right child is red and the new left child will then also be
 * red.
 */
static void rb_insert_rotate_left(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *tmp;

	tmp = node->right;
	node->right = tmp->left;
	tmp->left = node;

	/* fix colors and parent entries
	 * node must stay red during rotate
	 */
	rb_rotate_switch_parents(tmp, node, node->right, root, RB_RED);
}

/**
 * rb_insert_rotate_right() - Rotate at @node to the right
 * @node: pointer to the new node
 * @root: pointer to rb root
 *
 * The left child of @node is moved up the tree and the @node becomes the
 * right child of this tree. The right node is assumed to be black (or non
 * existing). The left child is red and the new right child will then also be
 * red.
 */
static void rb_insert_rotate_right(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *tmp;

	tmp = node->left;
	node->left = tmp->right;
	tmp->right = node;

	/* fix colors and parent entries
	 * node must stay red during rotate
	 */
	rb_rotate_switch_parents(tmp, node, node->left, root, RB_RED);
}

/**
 * rb_insert_color() - Go tree upwards and rebalance it after insert
 * @node: pointer to the new node
 * @root: pointer to rb root
 *
 * The tree is traversed from bottom to the top starting at the parent of
 * @node. The rebalancing is done via recoloring and rotating
 *
 * When the tree was an RB tree before the link of the new node then the
 * resulting tree will again be an RB tree
 */
void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *gparent;
	struct rb_node *parent;
	struct rb_node *uncle;
	bool uncle_is_right;

	while (1) {
		parent = rb_parent(node);

		/* reached root, mark it black */
		if (!parent) {
			rb_set_parent_color(node, NULL, RB_BLACK);
			break;
		}

		/* stop when no more fixes required on red path */
		if (rb_color(parent) == RB_BLACK)
			break;

		/* if parent is not black then node must have a black
		 * grandparent and might even have an uncle (for recoloring or
		 * rotating)
		 */
		gparent = rb_parent(parent);
		if (parent != gparent->right) {
			uncle_is_right = true;
			uncle = gparent->right;
		} else {
			uncle_is_right = false;
			uncle = gparent->left;
		}

		if (rb_is_red(uncle)) {
			/* switch colors of parent and uncle to black to avoid
			 * problem of two consecutive red nodes (node + parent)
			 */
			rb_insert_recolor(gparent);

			/* problem of new red node was just moved to grandparent
			 * and check has to continue there
			 */
			node = gparent;
		} else {
			/* the uncle either doesn't exist or is black. The two
			 * red nodes (node + parent) can be rotated together
			 * with the grandparent to form an standard 4-node
			 * (one red left child one red right child under a black
			 * node)
			 */
			if (uncle_is_right) {
				/* the red parent is left from the grandparent
				 * and the sibling is either black or doens't
				 * exist. The red colored child of parent must
				 * also be left or otherwise the rotation on
				 * the grandparent will create two consecutive
				 * red nodes on the right side of the subtree
				 */
				if (parent->right == node)
					rb_insert_rotate_left(parent, root);

				/* the black grandparent has no or black child
				 * on the right side. And the left child and
				 * the left most grandchild of it are red.
				 * A rotation to the right will then create
				 * a balanced 4-node (black node with two red
				 * children)
				 */
				rb_insert_rotate_right(gparent, root);
			} else {
				/* the red parent is right from the grandparent
				 * and the sibling is either black or doens't
				 * exist. The red colored child of parent must
				 * also be right or otherwise the rotation on
				 * the grandparent will create two consecutive
				 * red nodes on the left side of the subtree
				 */
				if (parent->left == node)
					rb_insert_rotate_right(parent, root);

				/* the black grandparent has no or black child
				 * on the left side. And the right child and
				 * the right most grandchild of it are red.
				 * A rotation to the left will then create
				 * a balanced 4-node (black node with two red
				 * children)
				 */
				rb_insert_rotate_left(gparent, root);
			}

			/* tree is now rebalanced again */
			break;
		}
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
	struct rb_node *smallest_right;
	enum rb_node_color smallest_color;

	if (!node->left && !node->right) {
		/* no child
		 * just delete the current child
		 */
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
	} else if (node->left && !node->right) {
		/* one child, left
		 * use left child as replacement for the deleted node
		 */
		rb_set_parent_color(node->left, rb_parent(node), RB_BLACK);
		rb_change_child(node, node->left, rb_parent(node), root);

		/* the left child must be red when there is no right child
		 * (3-node). Otherwise the subtrees would have different
		 * heights. So the child gets black and the red link is dropped.
		 * Nothing to rebalance anymore
		 */
		return NULL;
	} else if (!node->left) {
		/* one child, right
		 * use right child as replacement for the deleted node
		 */
		rb_set_parent_color(node->right, rb_parent(node), RB_BLACK);
		rb_change_child(node, node->right, rb_parent(node), root);

		/* the right child must be red when there is no left child
		 * (3-node). Otherwise the subtrees would have different
		 * heights.
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

	/* move right child of smallest one up */
	smallest_right = smallest->right;
	if (smallest_right)
		rb_set_parent_color(smallest_right, smallest_parent, RB_BLACK);
	rb_change_child(smallest, smallest_right, smallest_parent, root);

	/* exchange node with smallest */
	rb_set_parent_color(smallest, rb_parent(node), rb_color(node));

	smallest->left = node->left;
	rb_set_parent(smallest->left, smallest);

	smallest->right = node->right;
	if (smallest->right)
		rb_set_parent(smallest->right, smallest);

	rb_change_child(node, smallest, rb_parent(node), root);

	/* a red node can be ignored because the parent is a 3/4 node
	 * which gets then converted to 2/4 node after smallest node is moved
	 * up. If it is black then the parent node might get double black and
	 * thus has to be rebalanced
	 */
	if (smallest_color == RB_RED || smallest_right)
		return NULL;
	else
		return dblack;
}

/**
 * rb_erase_parent_swap_red_sibling_right() - Rotate at @parent to the left
 *  when right sibling is red
 * @parent: pointer to the new node
 * @root: pointer to rb root
 *
 * red sibling's tree has one layer of black nodes more than current tree.
 * parent must therefore be black.
 *
 * rotate the subtree under parent to the left (towards us). Sibling will become
 * the new "parent" of the subtree and the old parent is now the red node. The
 * imbalance between parent->left (-1) and parent->right didn't change and has
 * to be processed further.
 */
static void rb_erase_parent_swap_red_sibling_right(struct rb_node *parent,
						   struct rb_root *root)
{
	struct rb_node *tmp;

	if (!rb_is_red(parent->right))
		return;

	tmp = parent->right;
	parent->right = tmp->left;
	tmp->left = parent;

	rb_rotate_switch_parents(tmp, parent, parent->right, root, RB_RED);
}

/**
 * rb_erase_parent_swap_red_sibling_left() - Rotate at @parent to the right
 *  when left sibling is red
 * @parent: pointer to the new node
 * @root: pointer to rb root
 *
 * red sibling's tree has one layer of black nodes more than current tree.
 * parent must therefore be black.
 *
 * rotate the subtree under parent to the right (towards us). Sibling will
 * become the new "parent" of the subtree and the old parent is now the red
 * node. The imbalance between parent->right (-1) and parent->left didn't change
 * and has to be processed further.
 */
static void rb_erase_parent_swap_red_sibling_left(struct rb_node *parent,
						  struct rb_root *root)
{
	struct rb_node *tmp;

	if (!rb_is_red(parent->left))
		return;

	tmp = parent->left;
	parent->left = tmp->right;
	tmp->right = parent;

	rb_rotate_switch_parents(tmp, parent, parent->left, root, RB_RED);
}

/**
 * rb_erase_recolor() - Rebalance subtree via recolor
 * @parent: parent of subtree with imbalance
 * @sibling: pointer to rb root
 * @coming_from_right: pointer variable which stores which child of returned
 *  @parent caused imbalance
 *
 * Return: new double black @parent node, NULL otherwise
 */
static struct rb_node *rb_erase_recolor(struct rb_node *parent,
					struct rb_node *sibling,
					bool *coming_from_right)
{
	struct rb_node *gparent;

	rb_set_parent_color(sibling, parent, RB_RED);
	if (rb_is_red(parent)) {
		rb_set_color(parent, RB_BLACK);
		return NULL;
	} else {
		gparent = rb_parent(parent);
		if (gparent) {
			if (gparent->left == parent)
				*coming_from_right = false;
			else
				*coming_from_right = true;

			return gparent;
		} else {
			return NULL;
		}
	}
}

/**
 * rb_erase_color() - Go tree upwards and rebalance it after erase_node
 * @parent: double black node which has to be rebalanced after child was removed
 * @root: pointer to rb root
 *
 * The tree is traversed from bottom to the top starting at @parent. The
 * rebalancing is done via restructuring, recoloring and adjustment.
 *
 * When the tree was an RB tree before the erase of the node then the resulting
 * tree will again be an RB tree
 */
void rb_erase_color(struct rb_node *parent, struct rb_root *root)
{
	bool coming_from_right = false;
	struct rb_node *sibling;
	struct rb_node *tmp;

	/* the right child was removed when it is missing
	 * otherwise the left child was the smallest during erase and thus
	 * it was removed
	 */
	if (!parent->right)
		coming_from_right = true;

	/* go tree upwards and fix the nodes on the way */
	while (1) {

		if (!coming_from_right) {
			rb_erase_parent_swap_red_sibling_right(parent, root);

			sibling = parent->right;
			if (!sibling->right || !rb_is_red(sibling->right)) {
				if (!sibling->left ||
				    !rb_is_red(sibling->left)) {
					parent = rb_erase_recolor(parent,
								  sibling,
								  &coming_from_right);
					if (parent)
						continue;
					else
						break;
				} else {
					/* rotate sibling to the right. This
					 * moves the left red child to the
					 * right without changing the black
					 * height of any subtree
					 */
					tmp = sibling->left;
					sibling->left = tmp->right;
					tmp->right = sibling;

					rb_rotate_switch_parents(tmp, sibling,
								 sibling->left,
								 root, RB_RED);
				}
			}

			/* left rotate parent and recolor new sibling of
			 * parent
			 */
			tmp = parent->right;
			parent->right = tmp->left;
			tmp->left = parent;

			/* fix colors and parent entries
			 * node must stay black during rotate.
			 *
			 * The right (red) child of the sibling must become
			 * black to adjust the black height
			 */
			rb_rotate_switch_parents(tmp, parent, parent->right,
						 root, RB_BLACK);
			rb_set_color(tmp->right, RB_BLACK);

			break;
		} else {
			rb_erase_parent_swap_red_sibling_left(parent, root);

			sibling = parent->left;
			if (!sibling->left || !rb_is_red(sibling->left)) {
				if (!sibling->right ||
				    !rb_is_red(sibling->right)) {
					parent = rb_erase_recolor(parent,
								  sibling,
								  &coming_from_right);
					if (parent)
						continue;
					else
						break;
				} else {
					/* rotate sibling to the left. This
					 * moves the right red child to the
					 * left without changing the black
					 * height of any subtree
					 */
					tmp = sibling->right;
					sibling->right = tmp->left;
					tmp->left = sibling;

					rb_rotate_switch_parents(tmp, sibling,
								 sibling->right,
								 root, RB_RED);
				}
			}

			/* right rotate parent and recolor new sibling of
			 * parent
			 */
			tmp = parent->left;
			parent->left = tmp->right;
			tmp->right = parent;

			/* fix colors and parent entries
			 * node must stay black during rotate.
			 *
			 * The left (red) child of the sibling must become black
			 * to adjust the black height
			 */
			rb_rotate_switch_parents(tmp, parent, parent->left,
						 root, RB_BLACK);
			rb_set_color(tmp->left, RB_BLACK);

			break;
		}
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

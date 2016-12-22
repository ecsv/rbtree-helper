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

#ifndef __RBTREE_H__
#define __RBTREE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* inject the color info in the lowest bit of the parent pointer  */
#define RB_PARENT_COLOR_COMBINATION

#if defined(__GNUC__)
#define RBTREE_TYPEOF_USE 1
#define RB_NODE_ALIGNED __attribute__ ((aligned(sizeof(unsigned long))))
#endif

#if defined(_MSC_VER)
#define __inline__ __inline
#define RB_NODE_ALIGNED __declspec(align(sizeof(unsigned long)))
#endif

/**
 * container_of() - Calculate address of object that contains address ptr
 * @ptr: pointer to member variable
 * @type: type of the structure containing ptr
 * @member: name of the member variable in struct @type
 *
 * Return: @type pointer of object containing ptr
 */
#ifndef container_of
#ifdef RBTREE_TYPEOF_USE
#define container_of(ptr, type, member) __extension__ ({ \
	const __typeof__(((type *)0)->member) *__pmember = (ptr); \
	(type *)((char *)__pmember - offsetof(type, member)); })
#else
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))
#endif
#endif

/**
 * enum rb_node_color - type of node in rb_tree
 * @RB_RED: node is red (helper node to extend black node)
 * @RB_BLACK: node is black
 */
enum rb_node_color {
	RB_RED = 0,
	RB_BLACK = 1
};

/**
 * struct rb_node - node of a red-black tree
 * @parent: pointer to the parent node in the tree
 * @color: color of the node
 * @parent_color: combination of @parent and @color (lowest bit)
 * @left: pointer to the left child in the tree
 * @right: pointer to the right child in the tree
 *
 * The red-black tree consists of a root and nodes attached to this root. The
 * rb_* functions and macros can be used to access and modify this data
 * structure.
 *
 * The @parent pointer of the rb node points to the parent rb node in the
 * tree, the @left to the left "smaller key" child and @right to the right
 * "larger key" node of the tree.
 *
 * The rb nodes are usually embedded in a container structure which holds the
 * actual data. Such an container object is called entry. The helper rb_entry
 * can be used to calculate the object address from the address of the node.
 */
struct rb_node {
#ifndef RB_PARENT_COLOR_COMBINATION
	struct rb_node *parent;
	enum rb_node_color color;
#else
	unsigned long parent_color;
#endif
	struct rb_node *left;
	struct rb_node *right;
} RB_NODE_ALIGNED;

/**
 * struct rb_root - root of a red-black-tree
 * @node: pointer to the root node in the tree
 *
 * For an empty tree, node points to NULL.
 */
struct rb_root {
	struct rb_node *node;
};

/**
 * DEFINE_RBROOT - define tree root and initialize it
 * @root: name of the new object
 */
#define DEFINE_RBROOT(root) \
	struct rb_root root = { NULL }

/**
 * INIT_RB_ROOT() - Initialize empty tree
 * @root: pointer to rb root
 */
static __inline__ void INIT_RB_ROOT(struct rb_root *root)
{
	root->node = NULL;
}

/**
 * rb_empty() - Check if tree has no nodes attached
 * @root: pointer to the root of the tree
 *
 * Return: 0 - tree is not empty !0 - tree is empty
 */
static __inline__ int rb_empty(const struct rb_root *root)
{
	return !root->node;
}

/**
 * rb_parent() - Get parent of node
 * @node: pointer to the rb node
 *
 * Return: rb parent node of @node
 */
static __inline__ struct rb_node *rb_parent(struct rb_node *node)
{
#ifndef RB_PARENT_COLOR_COMBINATION
	return node->parent;
#else
	return (struct rb_node *)(node->parent_color & ~1lu);
#endif
}

/**
 * rb_color() - Get color of node
 * @node: pointer to the rb node
 *
 * Return: color of @node
 */
static __inline__ enum rb_node_color rb_color(const struct rb_node *node)
{
#ifndef RB_PARENT_COLOR_COMBINATION
	return node->color;
#else
	return (enum rb_node_color)(node->parent_color & 1lu);
#endif
}

void rb_insert(struct rb_node *node, struct rb_node *parent,
	       struct rb_node **rb_link, struct rb_root *root);
void rb_erase(struct rb_node *node, struct rb_root *root);

struct rb_node *rb_first(const struct rb_root *root);
struct rb_node *rb_last(const struct rb_root *root);
struct rb_node *rb_next(struct rb_node *node);
struct rb_node *rb_prev(struct rb_node *node);

/**
 * rb_entry() - Calculate address of entry that contains tree node
 * @node: pointer to tree node
 * @type: type of the entry containing the tree node
 * @member: name of the rb_node member variable in struct @type
 *
 * Return: @type pointer of entry containing node
 */
#define rb_entry(node, type, member) container_of(node, type, member)

#ifdef __cplusplus
}
#endif

#endif /* __RBTREE_H__ */

// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "../rbtree.h"
#include "common.h"
#include "common-treeops.h"

static uint16_t values[256];

static struct rbitem items[ARRAY_SIZE(values)];

int main(void)
{
	struct rb_root root;
	struct rb_node *node;
	struct rbitem *item;
	size_t i, j;

	INIT_RB_ROOT(&root);
	items[0].i = 0;
	rbitem_insert(&root, &items[0]);
	assert(rb_next(&items[0].rb) == NULL);

	for (i = 0; i < 256; i++) {
		random_shuffle_array(values, (uint16_t)ARRAY_SIZE(values));

		INIT_RB_ROOT(&root);
		node = rb_first(&root);
		assert(!node);

		for (j = 0; j < ARRAY_SIZE(values); j++) {
			items[j].i = values[j];
			rbitem_insert(&root, &items[j]);
		}

		for (node = rb_first(&root), j = 0;
		     node;
		     j++, node = rb_next(node)) {
			item = rb_entry(node, struct rbitem, rb);
			assert(item->i == j);
		}
		assert(j == ARRAY_SIZE(values));
		assert(!node);
	}

	return 0;
}

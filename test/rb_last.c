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
	uint16_t maxval = 0;

	for (i = 0; i < 256; i++) {
		random_shuffle_array(values, (uint16_t)ARRAY_SIZE(values));

		INIT_RB_ROOT(&root);
		node = rb_first(&root);
		assert(!node);

		for (j = 0; j < ARRAY_SIZE(values); j++) {
			if (j == 0)
				maxval = values[j];

			if (maxval < values[j])
				maxval = values[j];

			items[j].i = values[j];
			rbitem_insert_unbalanced(&root, &items[j]);

			node = rb_last(&root);
			assert(node);

			item = rb_entry(node, struct rbitem, rb);
			assert(item->i == maxval);
		}
	}

	return 0;
}

// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../rbtree.h"
#include "common.h"
#include "common-treeops.h"
#include "common-treevalidation.h"

static uint16_t values[256];
static uint16_t delete_items[ARRAY_SIZE(values)];
static uint8_t skiplist[ARRAY_SIZE(values)];

int main(void)
{
	struct rb_root root;
	size_t i, j;
	struct rbitem *item;

	for (i = 0; i < 256; i++) {
		random_shuffle_array(values, (uint16_t)ARRAY_SIZE(values));
		memset(skiplist, 1, sizeof(skiplist));

		INIT_RB_ROOT(&root);
		for (j = 0; j < ARRAY_SIZE(values); j++) {
			item = (struct rbitem *)malloc(sizeof(*item));
			assert(item);

			item->i = values[j];
			rbitem_insert_balanced(&root, item);
			skiplist[values[j]] = 0;
		}

		random_shuffle_array(delete_items, (uint16_t)ARRAY_SIZE(delete_items));
		for (j = 0; j < ARRAY_SIZE(delete_items); j++) {
			item = rbitem_find(&root, delete_items[j]);

			assert(item);
			assert(item->i == delete_items[j]);

			rb_erase(&item->rb, &root);
			skiplist[item->i] = 1;
			free(item);

			check_root_order(&root, skiplist,
					(uint16_t)ARRAY_SIZE(skiplist));
			check_depth(&root);
			check_rb_nodes(&root);
		}
		assert(rb_empty(&root));
	}

	return 0;
}

// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../rbtree.h"
#include "common.h"
#include "common-treeops.h"
#include "common-treevalidation.h"

static uint16_t values[256];

static struct rbitem items[ARRAY_SIZE(values)];
static uint8_t skiplist[ARRAY_SIZE(values)];

int main(void)
{
	struct rb_root root;
	size_t i, j;

	for (i = 0; i < 256; i++) {
		random_shuffle_array(values, (uint16_t)ARRAY_SIZE(values));
		memset(skiplist, 1, sizeof(skiplist));

		INIT_RB_ROOT(&root);
		for (j = 0; j < ARRAY_SIZE(values); j++) {
			items[j].i = values[j];
			rbitem_insert_unbalanced(&root, &items[j]);
			skiplist[values[j]] = 0;
			rb_insert_color(&items[j].rb, &root);

			check_root_order(&root, skiplist,
					 (uint16_t)ARRAY_SIZE(skiplist));
			check_depth(&root);
			check_rb_nodes(&root);
		}
	}

	return 0;
}

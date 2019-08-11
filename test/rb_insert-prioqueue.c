// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: 2012-2016, Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "../rbtree.h"
#include "common.h"
#include "common-prioqueue.h"

static uint16_t values[256];

int main(void)
{
	struct rb_prioqueue queue;
	size_t i, j;
	struct rbitem *item;

	for (i = 0; i < 256; i++) {
		random_shuffle_array(values, (uint16_t)ARRAY_SIZE(values));

		rb_prioqueue_init(&queue);
		for (j = 0; j < ARRAY_SIZE(values); j++) {
			item = (struct rbitem *)malloc(sizeof(*item));
			assert(item);

			item->i = values[j];
			rb_prioqueue_insert(&queue, item);
		}

		for (j = 0; j < ARRAY_SIZE(values); j++) {
			item = rb_prioqueue_pop(&queue);
			assert(item);
			assert(item->i == j);

			free(item);
		}
		assert(rb_empty(&queue.root));
	}

	return 0;
}

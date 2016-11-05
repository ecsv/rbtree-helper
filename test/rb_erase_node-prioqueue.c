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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "../rbtree.h"
#include "common.h"
#include "common-prioqueue.h"

static uint16_t values[256];
static uint16_t inserted;

static uint16_t valuequeue[ARRAY_SIZE(values)];
static uint16_t queuelen;

static uint16_t valuequeue_getmin(void)
{
	size_t i;
	uint16_t t;
	uint16_t *min_pos = NULL;

	for (i = 0; i < queuelen; i++) {
		if (!min_pos || valuequeue[i] < *min_pos)
			min_pos = &valuequeue[i];
	}

	if (!min_pos)
		return 0;

	t = valuequeue[queuelen - 1];
	valuequeue[queuelen - 1] = *min_pos;
	*min_pos = t;

	return valuequeue[queuelen - 1];
}

int main(void)
{
	struct rb_prioqueue queue;
	size_t i;
	struct rbitem *item;
	uint16_t operation;

	for (i = 0; i < 256; i++) {
		random_shuffle_array(values, (uint16_t)ARRAY_SIZE(values));
		inserted = 0;
		queuelen = 0;

		rb_prioqueue_init(&queue);
		while (inserted < ARRAY_SIZE(values) ||
		       queuelen != 0) {

			if (inserted == ARRAY_SIZE(values))
				operation = 0;
			else
				operation = get_unsigned16() % 2;

			if (operation == 1) {
				item = (struct rbitem *)malloc(sizeof(*item));
				assert(item);

				item->i = values[inserted];
				rb_prioqueue_insert_unbalanced(&queue, item);

				valuequeue[queuelen] = values[inserted];
				inserted++;
				queuelen++;
			} else {
				item = rb_prioqueue_pop_unbalanced(&queue);

				if (queuelen) {
					assert(item);
					assert(item->i == valuequeue_getmin());

					queuelen--;
					free(item);
				} else {
					assert(!item);
				}

			}
		}
		assert(rb_empty(&queue.root));
	}

	return 0;
}

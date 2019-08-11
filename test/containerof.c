// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: 2012-2016, Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>

#include "../rbtree.h"

struct teststruct {
	int a;
	int b;
};

int main(void)
{
	struct teststruct item;

	assert(&item == container_of(&item.b, struct teststruct, b));

	return 0;
}

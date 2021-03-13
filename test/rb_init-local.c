// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>

#include "../rbtree.h"

int main(void)
{
	DEFINE_RBROOT(testtree);
	assert(rb_empty(&testtree));

	return 0;
}

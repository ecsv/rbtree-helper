// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>

#include "../rbtree.h"

static DEFINE_RBROOT(testtree);

int main(void)
{
	assert(rb_empty(&testtree));

	return 0;
}

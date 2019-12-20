// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: 2012-2016, Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>

#include "../rbtree.h"

static DEFINE_RBROOT(testtree);

int main(void)
{
	assert(rb_empty(&testtree));

	return 0;
}

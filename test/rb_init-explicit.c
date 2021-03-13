// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>

#include "../rbtree.h"

int main(void)
{
	struct rb_root testtree;

	INIT_RB_ROOT(&testtree);
	assert(rb_empty(&testtree));

	return 0;
}

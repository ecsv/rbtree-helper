// SPDX-License-Identifier: MIT
/* Minimal red-black-tree helper functions test
 *
 * SPDX-FileCopyrightText: 2012-2016, Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>

#include "../rbtree.h"
#include "common.h"

int main(void)
{
	struct rbitem item;

	assert(&item == rb_entry(&item.rb, struct rbitem, rb));

	return 0;
}

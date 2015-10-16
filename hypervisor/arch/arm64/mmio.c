/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 *
 * Authors:
 *  Antonios Motakis <antonios.motakis@huawei.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <jailhouse/entry.h>
#include <jailhouse/mmio.h>
#include <jailhouse/printk.h>

unsigned int arch_mmio_count_regions(struct cell *cell)
{
	/* not entirely a lie :) */
	return 0;
}

void arm_mmio_perform_access(unsigned long base, struct mmio_access *mmio)
{
	trace_error(-EINVAL);
	while (1);
}

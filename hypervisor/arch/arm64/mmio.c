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

#include <jailhouse/mmio.h>
#include <jailhouse/printk.h>

unsigned int arch_mmio_count_regions(struct cell *cell)
{
	/* not entirely a lie :) */
	return 0;
}

void arm_mmio_perform_access(unsigned long base, struct mmio_access *mmio)
{
	void *addr = (void *)(base + mmio->address);

	if (mmio->is_write)
		switch (mmio->size) {
		case 1:
			mmio_write8(addr, mmio->value);
			return;
		case 2:
			mmio_write16(addr, mmio->value);
			return;
		case 4:
			mmio_write32(addr, mmio->value);
			return;
		}
	else
		switch (mmio->size) {
		case 1:
			mmio->value = mmio_read8(addr);
			return;
		case 2:
			mmio->value = mmio_read16(addr);
			return;
		case 4:
			mmio->value = mmio_read32(addr);
			return;
		}

	printk("WARNING: Ignoring unsupported MMIO access size %d\n",
	       mmio->size);
}

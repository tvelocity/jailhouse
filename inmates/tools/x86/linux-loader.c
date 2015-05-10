/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) Siemens AG, 2015
 *
 * Authors:
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <inmate.h>

struct boot_params {
	u8	padding1[0x230];
	u32	kernel_alignment;
	u8	padding2[0x260 - 0x230 - 4];
	u32	init_size;
};

#define ZERO_PAGE_ADDR		0xf5000UL

void inmate_main(void)
{
	struct boot_params *boot_params= (struct boot_params *)ZERO_PAGE_ADDR;
	void (*entry)(int, struct boot_params *);
	void *kernel;

	kernel = (void *)(unsigned long)boot_params->kernel_alignment;

	map_range(kernel, boot_params->init_size, MAP_CACHED);

	entry = kernel + 0x200;
	entry(0, boot_params);
}

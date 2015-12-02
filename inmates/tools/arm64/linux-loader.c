/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 *
 * Authors:
 *  Dmitry Voytik <dmitry.voytik@huawei.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <inmates/inmate.h>

/* Example memory map:
 *     0x00000000 - 0x00003fff (16K) this binary
 *     0x00004000 - 0x0000400f (16)  linux_cfg
 *     0x00280000                    Image
 *     0x0fe00000                    dtb
 */

#define LINUX_CFG_PADDR		0x4000UL

struct arm64_linux_header {
	u32 code0;		/* Executable code */
	u32 code1;		/* Executable code */
	u64 text_offset;	/* Image load offset, little endian */
	u64 image_size;		/* Effective Image size, little endian */
	u64 flags;		/* kernel flags, little endian */
	u64 res2;		/* = 0, reserved */
	u64 res3;		/* = 0, reserved */
	u64 res4;		/* = 0, reserved */
	u32 magic;		/* 0x644d5241 Magic number, little endian,
				   "ARM\x64" */
	u32 res5;		/* reserved (used for PE COFF offset) */
};

struct linux_cfg {
	unsigned long			dtb_addr;
	struct arm64_linux_header	*kernel_header;
};

void inmate_main(void)
{
	struct linux_cfg *lin_cfg;
	void (*entry)(unsigned long);
	u64 kaddr;

	lin_cfg = (struct linux_cfg*)LINUX_CFG_PADDR;

	kaddr = (u64)lin_cfg->kernel_header;

	entry = (void*)(unsigned long)kaddr;

	printk("\nJailhouse ARM64 Linux bootloader\n");
	printk("DTB:        0x%016lx\n", lin_cfg->dtb_addr);
	printk("Image:      0x%016lx\n", lin_cfg->kernel_header);
	printk("Image size: %lu Bytes\n", lin_cfg->kernel_header->image_size);
	printk("entry:      0x%016lx\n", entry);
	if (lin_cfg->kernel_header->magic != 0x644d5241)
		printk("WARNING: wrong Linux Image header magic: 0x%08x\n",
		       lin_cfg->kernel_header->magic);

	entry(lin_cfg->dtb_addr);
}

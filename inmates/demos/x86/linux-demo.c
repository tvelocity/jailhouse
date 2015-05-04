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

/* Linux boot parameters */
#include <asm/bootparam.h>

#ifdef CONFIG_UART_OXPCIE952
#define UART_BASE		0xe010
#else
#define UART_BASE		0x3f8
#endif

#define ZERO_PAGE_ADDR		0x010000UL
#define CMD_LINE_ADDR		0x020000UL
#define IMAGE_LOAD_ADDR		0x200000UL

static const char cmd_line[] =
	"earlyprintk=ttyS0 memmap=1M@0 memmap=0x3aff000@0x200000 "
	"console=ttyS0 pmtmr=0x608";

void inmate_main(void)
{
	struct setup_header *header = (void *)(IMAGE_LOAD_ADDR + 0x1f1);
	struct boot_params *boot_params= (void*)ZERO_PAGE_ADDR;
	const char *str = (void *)(IMAGE_LOAD_ADDR + 0x200);
	void (*entry)(int, struct boot_params *);
	unsigned long setup_size, prot_image_size;
	void *prot_image, *kernel;

	printk_uart_base = UART_BASE;
	printk("\nStarting Linux non-root cell\n\n");

	map_range((void *)IMAGE_LOAD_ADDR, 64 * 1024 * 1024, MAP_CACHED);

	str += header->kernel_version;
	printk("Kernel version:\t%s\n", str);

	printk("Relocatable:\t%d\n", header->relocatable_kernel);
	printk("Alignment:\t%x..%x\n",
	       (1 << header->min_alignment), header->kernel_alignment);

	setup_size = (header->setup_sects + 1) * 512;
	prot_image = (void *)(IMAGE_LOAD_ADDR + setup_size);
	printk("Image start:\t%x\n", (unsigned long)prot_image);
	prot_image_size = header->syssize * 16;
	printk("Image size:\t%x\n", prot_image_size);

	memset(boot_params, 0, sizeof(*boot_params));
	memcpy(&boot_params->hdr, header, 0x202 + (header->jump >> 8));

	boot_params->hdr.type_of_loader = 0xff;

	boot_params->hdr.cmd_line_ptr = (unsigned long)cmd_line;

	boot_params->hdr.ramdisk_image = 0x600000;
	boot_params->hdr.ramdisk_size = 2352271;

	kernel = (void *)(unsigned long)header->kernel_alignment;
	memcpy(kernel, prot_image, prot_image_size);

	entry = kernel + 0x200;
	entry(0, boot_params);
}

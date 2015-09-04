/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 * Copyright (C) 2014 ARM Limited
 *
 * Authors:
 *  Antonios Motakis <antonios.motakis@huawei.com>
 *
 * Part of the fuctionality is derived from the AArch32 implementation, under
 * hypervisor/arch/arm/mmio.c by Jean-Philippe Brucker.
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <jailhouse/entry.h>
#include <jailhouse/mmio.h>
#include <jailhouse/printk.h>
#include <asm/bitops.h>
#include <asm/percpu.h>
#include <asm/sysregs.h>
#include <asm/traps.h>

/* AARCH64_TODO: consider merging this with the AArch32 version */

unsigned int arch_mmio_count_regions(struct cell *cell)
{
	/* not entirely a lie :) */
	return 0;
}

static void arch_inject_dabt(struct trap_context *ctx, unsigned long addr)
{
	trace_error(-EINVAL);
	while(1);
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
		case 8:
			mmio_write64(addr, mmio->value);
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
		case 8:
			mmio->value = mmio_read64(addr);
			return;
		}

	printk("WARNING: Ignoring unsupported MMIO access size %d\n",
	       mmio->size);
}

int arch_handle_dabt(struct trap_context *ctx)
{
	enum mmio_result mmio_result;
	struct mmio_access mmio;
	unsigned long hpfar;
	unsigned long hdfar;
	/* Decode the syndrome fields */
	u32 icc		= ESR_ICC(ctx->esr);
	u32 isv		= icc >> 24;
	u32 sas		= icc >> 22 & 0x3;
	u32 sse		= icc >> 21 & 0x1;
	u32 srt		= icc >> 16 & 0xf;
	u32 ea		= icc >> 9 & 0x1;
	u32 cm		= icc >> 8 & 0x1;
	u32 s1ptw	= icc >> 7 & 0x1;
	u32 is_write	= icc >> 6 & 0x1;
	u32 size	= 1 << sas;

	arm_read_sysreg(HPFAR_EL2, hpfar);
	arm_read_sysreg(FAR_EL2, hdfar);
	mmio.address = hpfar << 8;
	mmio.address |= hdfar & 0xfff;

	this_cpu_data()->stats[JAILHOUSE_CPU_STAT_VMEXITS_MMIO]++;

	/*
	 * Invalid instruction syndrome means multiple access or writeback, there
	 * is nothing we can do.
	 */
	if (!isv || size > sizeof(unsigned long))
		goto error_unhandled;

	/* Re-inject abort during page walk, cache maintenance or external */
	if (s1ptw || ea || cm) {
		arch_inject_dabt(ctx, hdfar);
		return TRAP_HANDLED;
	}

	if (is_write) {
		/* Load the value to write from the src register */
		access_cell_reg(ctx, srt, &mmio.value, true);
		if (sse)
			mmio.value = sign_extend(mmio.value, 8 * size);
	} else {
		mmio.value = 0;
	}
	mmio.is_write = is_write;
	mmio.size = size;

	mmio_result = mmio_handle_access(&mmio);
	if (mmio_result == MMIO_ERROR)
		return TRAP_FORBIDDEN;
	if (mmio_result == MMIO_UNHANDLED)
		goto error_unhandled;

	/* Put the read value into the dest register */
	if (!is_write) {
		if (sse)
			mmio.value = sign_extend(mmio.value, 8 * size);
		access_cell_reg(ctx, srt, &mmio.value, false);
	}

	arch_skip_instruction(ctx);
	return TRAP_HANDLED;

error_unhandled:
	panic_printk("Unhandled data %s at 0x%x(%d)\n",
		(is_write ? "write" : "read"), mmio.address, size);

	return TRAP_UNHANDLED;
}

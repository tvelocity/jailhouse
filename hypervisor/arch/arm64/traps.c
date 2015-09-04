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

#include <jailhouse/control.h>
#include <jailhouse/printk.h>
#include <asm/control.h>
#include <asm/gic_common.h>
#include <asm/platform.h>
#include <asm/psci.h>
#include <asm/sysregs.h>
#include <asm/traps.h>

void arch_skip_instruction(struct trap_context *ctx)
{
	trace_error(-EINVAL);
	while(1);
}

void access_cell_reg(struct trap_context *ctx, u8 reg, unsigned long *val,
		     bool is_read)
{
	trace_error(-EINVAL);
	while(1);
}

void arch_handle_trap(struct per_cpu *cpu_data, struct registers *guest_regs)
{
	trace_error(-EINVAL);
	while(1);
}

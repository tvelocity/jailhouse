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
#include <asm/mmio.h>
#include <asm/platform.h>
#include <asm/psci.h>
#include <asm/sysregs.h>
#include <asm/traps.h>

void arch_skip_instruction(struct trap_context *ctx)
{
	u32 instruction_length = ESR_IL(ctx->esr);

	ctx->pc += (instruction_length ? 4 : 2);
}


void access_cell_reg(struct trap_context *ctx, u8 reg, unsigned long *val,
		     bool is_read)
{
	if (is_read)
		*val = ctx->regs[reg];
	else
		ctx->regs[reg] = *val;
}

static const trap_handler trap_handlers[38] =
{
	[ESR_EC_DABT]		= arch_handle_dabt,
};

void arch_handle_trap(struct per_cpu *cpu_data, struct registers *guest_regs)
{
	struct trap_context ctx;
	u32 exception_class;
	int ret = TRAP_UNHANDLED;

	arm_read_sysreg(ELR_EL2, ctx.pc);
	arm_read_sysreg(SPSR_EL2, ctx.cpsr);
	arm_read_sysreg(ESR_EL2, ctx.esr);
	exception_class = ESR_EC(ctx.esr);
	ctx.regs = guest_regs->usr;

	if (trap_handlers[exception_class])
		ret = trap_handlers[exception_class](&ctx);

	switch (ret) {
	case TRAP_UNHANDLED:
	case TRAP_FORBIDDEN:
		panic_printk("FATAL: %s (exception class 0x%02x)\n",
			     (ret == TRAP_UNHANDLED ? "unhandled trap" :
						      "forbidden access"),
			     exception_class);
		/* AARCH64_TODO: dump_guest_regs(&ctx); */
		panic_park();
	}

	arm_write_sysreg(ELR_EL2, ctx.pc);
}

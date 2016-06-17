/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 *
 * Authors:
 *  Antonios Motakis <antonios.motakis@huawei.com>
 *  Dmitry Voytik <dmitry.voytik@huawei.com>
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
#include <asm/processor.h>
#include <asm/irqchip.h>

void arch_skip_instruction(struct trap_context *ctx)
{
	u32 instruction_length = ESR_IL(ctx->esr);

	ctx->pc += (instruction_length ? 4 : 2);
}

static int arch_handle_smc(struct trap_context *ctx)
{
	unsigned long *regs = ctx->regs;

	if (IS_PSCI_32(regs[0]) || IS_PSCI_64(regs[0])) {
		regs[0] = psci_dispatch(ctx);
		arch_skip_instruction(ctx);

		return TRAP_HANDLED;
	}

	return TRAP_UNHANDLED;
}

static int arch_handle_hvc(struct trap_context *ctx)
{
	unsigned long *regs = ctx->regs;

	if (IS_PSCI_32(regs[0]) || IS_PSCI_64(regs[0])) {
		regs[0] = psci_dispatch(ctx);
		return TRAP_HANDLED;
	}

	return TRAP_UNHANDLED;
}

static void dump_regs(struct trap_context *ctx)
{
	unsigned char i;

	panic_printk(" pc: %016lx   lr: %016lx spsr: %08lx     EL%1d\n"
		     " sp: %016lx  esr: %02x %01x %07lx\n",
		     ctx->pc, ctx->regs[30], ctx->spsr, SPSR_EL(ctx->spsr),
		     ctx->sp, ESR_EC(ctx->esr), ESR_IL(ctx->esr),
		     ESR_ISS(ctx->esr));
	for (i = 0; i < NUM_USR_REGS - 1; i++)
		panic_printk("%sx%d: %016lx%s", i < 10 ? " " : "", i,
			     ctx->regs[i], i % 3 == 2 ? "\n" : "  ");
	panic_printk("\n");
}

/* TODO: move this function to an arch-independent code if other architectures
 * will need it.
 */
static void dump_mem(unsigned long start, unsigned long stop)
{
	unsigned long caddr = start & ~0x1f;

	if (stop <= start)
		return;
	printk("(0x%016lx - 0x%016lx):", start, stop);
	for (;;) {
		printk("\n%04lx: ", caddr & 0xffe0);
		do {
			if (caddr >= start)
				printk("%08x ", *(unsigned int *)caddr);
			else
				printk("         ", *(unsigned int *)caddr);
			caddr += 4;
		} while ((caddr & 0x1f) && caddr < stop);
		if (caddr >= stop)
			break;
	}
	printk("\n");
}

static void dump_hyp_stack(const struct trap_context *ctx)
{
	panic_printk("Hypervisor stack before exception ");
	dump_mem(ctx->sp, (unsigned long)this_cpu_data()->stack +
							PERCPU_STACK_END);
}

static void fill_trap_context(struct trap_context *ctx, struct registers *regs)
{
	arm_read_sysreg(ELR_EL2, ctx->pc);
	arm_read_sysreg(SPSR_EL2, ctx->spsr);
	switch (SPSR_EL(ctx->spsr)) {	/* exception level */
	case 0:
		arm_read_sysreg(SP_EL0, ctx->sp); break;
	case 1:
		arm_read_sysreg(SP_EL1, ctx->sp); break;
	case 2:
		/* SP_EL2 is not accessible in EL2. To obtain SP value before
		 * the excepton we can use the addres of *regs parameter.  *regs
		 * is located in the stack (see handle_vmexit in exception.S) */
		ctx->sp = (u64)(regs) + 16 * 16; break;
	default:
		ctx->sp = 0; break;	/* should never happen */
	}
	arm_read_sysreg(ESR_EL2, ctx->esr);
	ctx->regs = regs->usr;
}

static void arch_handle_trap(struct per_cpu *cpu_data,
			     struct registers *guest_regs)
{
	struct trap_context ctx;
	int ret;

	fill_trap_context(&ctx, guest_regs);

	/* exception class */
	switch (ESR_EC(ctx.esr)) {
	case ESR_EC_DABT_LOW:
		ret = arch_handle_dabt(&ctx);
		break;

	case ESR_EC_SMC64:
		ret = arch_handle_smc(&ctx);
		break;

	case ESR_EC_HVC64:
		ret = arch_handle_hvc(&ctx);
		break;

	default:
		ret = TRAP_UNHANDLED;
	}

	if (ret == TRAP_UNHANDLED || ret == TRAP_FORBIDDEN) {
		panic_printk("\nFATAL: exception %s\n", (ret == TRAP_UNHANDLED ?
							 "unhandled trap" :
							 "forbidden access"));
		panic_printk("Cell state before exception:\n");
		dump_regs(&ctx);
		panic_park();
	}

	arm_write_sysreg(ELR_EL2, ctx.pc);
}

static void arch_dump_exit(struct registers *regs, const char *reason)
{
	struct trap_context ctx;

	fill_trap_context(&ctx, regs);
	panic_printk("\nFATAL: Unhandled HYP exception: %s\n", reason);
	dump_regs(&ctx);
	dump_hyp_stack(&ctx);
}

struct registers *arch_handle_exit(struct per_cpu *cpu_data,
				   struct registers *regs)
{
	cpu_data->stats[JAILHOUSE_CPU_STAT_VMEXITS_TOTAL]++;

	switch (regs->exit_reason) {
	case EXIT_REASON_EL1_IRQ:
		irqchip_handle_irq(cpu_data);
		break;

	case EXIT_REASON_EL1_ABORT:
		arch_handle_trap(cpu_data, regs);
		break;

	case EXIT_REASON_EL2_ABORT:
		arch_dump_exit(regs, "synchronous abort from EL2");
		panic_stop();
		break;

	default:
		arch_dump_exit(regs, "unexpected");
		panic_stop();
	}

	vmreturn(regs);
}

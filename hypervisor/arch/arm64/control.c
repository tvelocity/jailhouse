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
#include <asm/traps.h>

int arch_cell_create(struct cell *cell)
{
	return trace_error(-EINVAL);
}

void arch_flush_cell_vcpu_caches(struct cell *cell)
{
}

void arch_cell_destroy(struct cell *cell)
{
	trace_error(-EINVAL);
	while(1);
}

void arch_config_commit(struct cell *cell_added_removed)
{
}

void arch_shutdown(void)
{
	trace_error(-EINVAL);
	while(1);
}

void arch_suspend_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while(1);
}

static void arch_dump_exit(struct registers *regs, const char *reason)
{
	unsigned long pc;
	unsigned int n;

	arm_read_sysreg(ELR_EL2, pc);
	panic_printk("Unhandled HYP %s exit at 0x%x\n", reason, pc);
	for (n = 0; n < NUM_USR_REGS; n++)
		panic_printk("r%d:%s 0x%08lx%s", n, n < 10 ? " " : "",
			     regs->usr[n], n % 4 == 3 ? "\n" : "  ");
	panic_printk("\n");
}

static void arch_handle_el2_trap(struct per_cpu *cpu_data,
				 struct registers *regs)
{
	unsigned long pc;
	unsigned long esr_el2;

	arm_read_sysreg(ELR_EL2, pc);
	arm_read_sysreg(ESR_EL2, esr_el2);

	panic_printk("Unhandled EL2 synchronous abort at 0x%x, "
		     "exception class 0x%02x\n", pc, ESR_EC(esr_el2));
}

struct registers* arch_handle_exit(struct per_cpu *cpu_data,
				   struct registers *regs)
{
	cpu_data->stats[JAILHOUSE_CPU_STAT_VMEXITS_TOTAL]++;

	switch (regs->exit_reason) {
	case EXIT_REASON_EL2_ABORT:
		arch_handle_el2_trap(cpu_data, regs);
		break;

	case EXIT_REASON_EL1_ABORT:
		arch_handle_trap(cpu_data, regs);
		break;

	default:
		arch_dump_exit(regs, "unknown");
		panic_stop();
	}

	/* AARCH64_TODO: shutdown support */

	return regs;
}

void arch_resume_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while(1);
}

void arch_reset_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while(1);
}

void arch_park_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while(1);
}

void arch_shutdown_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while(1);
}

void __attribute__((noreturn)) arch_panic_stop(void)
{
	trace_error(-EINVAL);
	while(1);
}

void arch_panic_park(void)
{
	trace_error(-EINVAL);
	while(1);
}

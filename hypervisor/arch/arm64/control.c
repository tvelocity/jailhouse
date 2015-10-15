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
#include <jailhouse/string.h>
#include <asm/control.h>
#include <asm/irqchip.h>
#include <asm/platform.h>
#include <asm/traps.h>

static void arch_reset_el1(struct registers *regs)
{
	/* put the cpu in a reset state */
	/* AARCH64_TODO: handle big endian support */
	arm_write_sysreg(SPSR_EL2, RESET_PSR);
	arm_write_sysreg(SCTLR_EL1, SCTLR_EL1_RES1);
	arm_write_sysreg(CNTKCTL_EL1, 0);
	arm_write_sysreg(PMCR_EL0, 0);

	/* wipe any other state to avoid leaking information accross cells */
	memset(regs, 0, sizeof(struct registers));

	/* AARCH64_TODO: wipe floating point registers */

	/* wipe special registers */
	arm_write_sysreg(SP_EL0, 0);
	arm_write_sysreg(SP_EL1, 0);
	arm_write_sysreg(SPSR_EL1, 0);

	/* wipe the system registers */
	arm_write_sysreg(AFSR0_EL1, 0);
	arm_write_sysreg(AFSR1_EL1, 0);
	arm_write_sysreg(AMAIR_EL1, 0);
	arm_write_sysreg(CONTEXTIDR_EL1, 0);
	arm_write_sysreg(CPACR_EL1, 0);
	arm_write_sysreg(CSSELR_EL1, 0);
	arm_write_sysreg(ESR_EL1, 0);
	arm_write_sysreg(FAR_EL1, 0);
	arm_write_sysreg(MAIR_EL1, 0);
	arm_write_sysreg(PAR_EL1, 0);
	arm_write_sysreg(TCR_EL1, 0);
	arm_write_sysreg(TPIDRRO_EL0, 0);
	arm_write_sysreg(TPIDR_EL0, 0);
	arm_write_sysreg(TPIDR_EL1, 0);
	arm_write_sysreg(TTBR0_EL1, 0);
	arm_write_sysreg(TTBR1_EL1, 0);
	arm_write_sysreg(VBAR_EL1, 0);

	/* wipe timer registers */
	arm_write_sysreg(CNTP_CTL_EL0, 0);
	arm_write_sysreg(CNTP_CVAL_EL0, 0);
	arm_write_sysreg(CNTP_TVAL_EL0, 0);
	arm_write_sysreg(CNTV_CTL_EL0, 0);
	arm_write_sysreg(CNTV_CVAL_EL0, 0);
	arm_write_sysreg(CNTV_TVAL_EL0, 0);

	/* AARCH64_TODO: handle PMU registers */
	/* AARCH64_TODO: handle debug registers */
	/* AARCH64_TODO: handle system registers for AArch32 state */
}

void arch_reset_self(struct per_cpu *cpu_data)
{
	int err = 0;
	unsigned long reset_address;
	struct cell *cell = cpu_data->cell;
	struct registers *regs = guest_regs(cpu_data);

	if (cell != &root_cell) {
		trace_error(-EINVAL);
		panic_stop();
	}

	/*
	 * On the first CPU to reach this, write all cell datas to memory so it
	 * can be started with caches disabled.
	 * On all CPUs, invalidate the instruction caches to take into account
	 * the potential new instructions.
	 */
	arch_cell_caches_flush(cell);

	err = irqchip_cpu_reset(cpu_data);
	if (err)
		printk("IRQ setup failed\n");

	/* Wait for the driver to call cpu_up */
	reset_address = psci_emulate_spin(cpu_data);

	/* Set the new MPIDR */
	arm_write_sysreg(VMPIDR_EL2, cpu_data->virt_id | MPIDR_MP_BIT);

	/* Restore an empty context */
	arch_reset_el1(regs);

	arm_write_sysreg(ELR_EL2, reset_address);

	vmreturn(regs);
}

int arch_cell_create(struct cell *cell)
{
	return trace_error(-EINVAL);
}

void arch_flush_cell_vcpu_caches(struct cell *cell)
{
	/* AARCH64_TODO */
	trace_error(-EINVAL);
}

void arch_cell_destroy(struct cell *cell)
{
	trace_error(-EINVAL);
	while (1);
}

void arch_config_commit(struct cell *cell_added_removed)
{
}

void arch_shutdown(void)
{
	trace_error(-EINVAL);
	while (1);
}

void arch_suspend_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while (1);
}

void arch_resume_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while (1);
}

void arch_reset_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while (1);
}

void arch_park_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while (1);
}

void arch_shutdown_cpu(unsigned int cpu_id)
{
	trace_error(-EINVAL);
	while (1);
}

void __attribute__((noreturn)) arch_panic_stop(void)
{
	trace_error(-EINVAL);
	while (1);
}

void arch_panic_park(void)
{
	trace_error(-EINVAL);
	while (1);
}

void arch_handle_sgi(struct per_cpu *cpu_data, u32 irqn)
{
	cpu_data->stats[JAILHOUSE_CPU_STAT_VMEXITS_MANAGEMENT]++;

	switch (irqn) {
	case SGI_INJECT:
		irqchip_inject_pending(cpu_data);
		break;
	default:
		printk("WARN: unknown SGI received %d\n", irqn);
	}
}

unsigned int arm_cpu_virt2phys(struct cell *cell, unsigned int virt_id)
{
	unsigned int cpu;

	for_each_cpu(cpu, cell->cpu_set) {
		if (per_cpu(cpu)->virt_id == virt_id)
			return cpu;
	}

	return -1;
}

unsigned int arm_cpu_phys2virt(unsigned int cpu_id)
{
	return per_cpu(cpu_id)->virt_id;
}

/*
 * Handle the maintenance interrupt, the rest is injected into the cell.
 * Return true when the IRQ has been handled by the hyp.
 */
bool arch_handle_phys_irq(struct per_cpu *cpu_data, u32 irqn)
{
	if (irqn == MAINTENANCE_IRQ) {
		cpu_data->stats[JAILHOUSE_CPU_STAT_VMEXITS_MAINTENANCE]++;

		irqchip_inject_pending(cpu_data);
		return true;
	}

	cpu_data->stats[JAILHOUSE_CPU_STAT_VMEXITS_VIRQ]++;

	irqchip_set_pending(cpu_data, irqn, true);

	return false;
}

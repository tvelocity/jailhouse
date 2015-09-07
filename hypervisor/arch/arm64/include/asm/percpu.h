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

#ifndef _JAILHOUSE_ASM_PERCPU_H
#define _JAILHOUSE_ASM_PERCPU_H

#include <jailhouse/types.h>
#include <asm/paging.h>

#define NUM_ENTRY_REGS			31

/* Keep in sync with struct per_cpu! */
#define PERCPU_SIZE_SHIFT		13
#define PERCPU_STACK_END		PAGE_SIZE
#define PERCPU_LINUX_SP			PERCPU_STACK_END

#ifndef __ASSEMBLY__

#include <jailhouse/printk.h>
#include <asm/cell.h>
#include <asm/spinlock.h>

struct pending_irq;

struct per_cpu {
	u8 stack[PAGE_SIZE];

	/* common fields */
	unsigned int cpu_id;
	struct cell *cell;
	u32 stats[JAILHOUSE_NUM_CPU_STATS];
	int shutdown_state;
	bool failed;

	/* Other CPUs can insert sgis into the pending array */
	spinlock_t gic_lock;
	struct pending_irq *pending_irqs;
	struct pending_irq *first_pending;
	/* Only GICv3: redistributor base */
	void *gicr_base;

	bool flush_vcpu_caches;
} __attribute__((aligned(PAGE_SIZE)));

static inline struct per_cpu *this_cpu_data(void)
{
	struct per_cpu *cpu_data;

	arm_read_sysreg(TPIDR_EL2, cpu_data);
	return cpu_data;
}

#define DEFINE_PER_CPU_ACCESSOR(field)					\
static inline typeof(((struct per_cpu *)0)->field) this_##field(void)	\
{									\
	return this_cpu_data()->field;					\
}

DEFINE_PER_CPU_ACCESSOR(cpu_id)
DEFINE_PER_CPU_ACCESSOR(cell)

static inline struct per_cpu *per_cpu(unsigned int cpu)
{
	extern u8 __page_pool[];

	return (struct per_cpu *)(__page_pool + (cpu << PERCPU_SIZE_SHIFT));
}

unsigned int arm_cpu_phys2virt(unsigned int cpu_id);
unsigned int arm_cpu_virt2phys(struct cell *cell, unsigned int virt_id);

#endif /* !__ASSEMBLY__ */

#endif /* !_JAILHOUSE_ASM_PERCPU_H */

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

#include <asm/cell.h>
#include <asm/spinlock.h>

struct per_cpu {
	u8 stack[PAGE_SIZE];
	unsigned long linux_sp;
	unsigned long linux_ret;
	unsigned long linux_flags;
	unsigned long linux_reg[NUM_ENTRY_REGS];

	/* common fields */
	unsigned int cpu_id;
	struct cell *cell;
	u32 stats[JAILHOUSE_NUM_CPU_STATS];
	int shutdown_state;
	bool failed;

	bool flush_vcpu_caches;
} __attribute__((aligned(PAGE_SIZE)));

static inline struct per_cpu *this_cpu_data(void)
{
	return NULL;
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
	return NULL;
}

#endif /* !__ASSEMBLY__ */

#endif /* !_JAILHOUSE_ASM_PERCPU_H */

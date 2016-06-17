/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) ARM Limited, 2014
 *
 * Authors:
 *  Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
#ifndef _JAILHOUSE_INMATES_GIC_H
#define _JAILHOUSE_INMATES_GIC_H

#include <inmate.h>

#define GICD_ISENABLER			0x0100

#define is_spi(irqn)			((irqn) > 31 && (irqn) < 1020)

#ifndef __ASSEMBLY__

static inline void gic_setup_irq_stack(void)
{
	static __attribute__((aligned(0x1000))) u32 irq_stack[1024];

	asm volatile (".arch_extension virt\n");
	asm volatile ("msr	SP_irq, %0\n" : : "r" (irq_stack));
	asm volatile ("cpsie	i\n");
}

int gic_init(void);
void gic_enable(unsigned int irqn);
void gic_write_eoi(u32 irqn);
u32 gic_read_ack(void);

#endif /* !__ASSEMBLY__ */
#endif

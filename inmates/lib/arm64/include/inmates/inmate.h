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
#ifndef _JAILHOUSE_INMATES_INMATE_H
#define _JAILHOUSE_INMATES_INMATE_H

#ifndef __ASSEMBLY__
typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

static inline void *memset(void *addr, int val, unsigned long size)
{
	char *s = addr;
	unsigned int i;
	for (i = 0; i < size; i++)
		*s++ = val;

	return addr;
}

extern unsigned long printk_uart_base;
void printk(const char *fmt, ...);
void inmate_main(void);

void __attribute__((used)) vector_irq(void);

typedef void (*irq_handler_t)(unsigned int);
void gic_setup(irq_handler_t handler);
void gic_enable_irq(unsigned int irq);

unsigned long timer_get_frequency(void);
u64 timer_get_ticks(void);
u64 timer_ticks_to_ns(u64 ticks);
void timer_start(u64 timeout);

#endif /* !__ASSEMBLY__ */
#endif

/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 *
 * Authors:
 *  Antonios Motakis <antonios.motakis@huawei.com>
 *  Claudio Fontana <claudio.fontana@huawei.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#ifndef _JAILHOUSE_ASM_BITOPS_H
#define _JAILHOUSE_ASM_BITOPS_H

#include <jailhouse/types.h>

#ifndef __ASSEMBLY__

#define BITOPT_ALIGN(bits, addr)				\
	do {							\
		(addr) = (unsigned long *)((u64)(addr) & ~0x7)	\
			+ (bits) / BITS_PER_LONG;		\
		(bits) %= BITS_PER_LONG;			\
	} while (0)

static inline __attribute__((always_inline)) void
clear_bit(int nr, volatile unsigned long *addr)
{
	u32 ret;

	BITOPT_ALIGN(nr, addr);

	/* AARCH64_TODO: do we need to preload? */
	do {
		asm volatile (
			"ldxr	x3, %1\n\t"
			"eor	x3, x3, %2\n\t"
			"stxr	%w0, x3, %1\n\t"
			: "=r" (ret),
			  "+Q" (*(volatile unsigned long *)addr)
			: "r" (1 << nr)
			: "x3");
	} while (ret);
}

static inline __attribute__((always_inline)) void
set_bit(unsigned int nr, volatile unsigned long *addr)
{
	u32 ret;

	BITOPT_ALIGN(nr, addr);

	/* AARCH64_TODO: do we need to preload? */
	do {
		asm volatile (
			"ldxr	x3, %1\n\t"
			"orr	x3, x3, %2\n\t"
			"stxr	%w0, x3, %1\n\t"
			: "=r" (ret),
			  "+Q" (*(volatile unsigned long *)addr)
			: "r" (1 << nr)
			: "x3");
	} while (ret);
}

static inline __attribute__((always_inline)) int
test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
}

static inline int test_and_set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long ret = 0, test;

	do {
		asm volatile (
			"ldxr	x3, %2\n\t"
			"ands	%1, x3, %3\n\t"
			"b.ne	1f \n\t"
			"orr	x3, x3, %3\n\t"
			"stxr	%w0, x3, %2\n\t"
			"1:\n\t"
			: "=r" (ret), "=&r" (test),
			  "+Q" (*(volatile unsigned long *)addr)
			: "r" (1 << nr)
			: "x3");
	} while (ret);
	return !!(test);
}

/* Count leading zeroes */
static inline unsigned long clz(unsigned long word)
{
	unsigned long val;
	asm volatile ("clz %0, %1" : "=r" (val) : "r" (word));
	return val;
}

/* Returns the position of the least significant 1, MSB=63, LSB=0*/
static inline unsigned long ffsl(unsigned long word)
{
	if (!word)
		return 0;
	asm volatile ("rbit %0, %0" : "+r" (word));
	return clz(word);
}

static inline unsigned long ffzl(unsigned long word)
{
	return ffsl(~word);
}

/* AARCH64_TODO: we can use SXTB, SXTH, SXTW */
/* Extend the value of 'size' bits to a signed long */
static inline unsigned long sign_extend(unsigned long val, unsigned int size)
{
	unsigned long mask;

	if (size >= sizeof(unsigned long) * 8)
		return val;

	mask = 1U << (size - 1);
	return (val ^ mask) - mask;
}

#endif /* !__ASSEMBLY__ */
#endif /* !_JAILHOUSE_ASM_BITOPS_H */

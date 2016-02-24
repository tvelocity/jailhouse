/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 *
 * Authors:
 *  Antonios Motakis <antonios.motakis@huawei.com>
 *
 * Spinlock implementation copied from
 * arch/arm64/include/asm/spinlock.h in Linux
 *
 * Copyright (C) 2012 ARM Ltd.
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */

#ifndef _JAILHOUSE_ASM_SPINLOCK_H
#define _JAILHOUSE_ASM_SPINLOCK_H

#define DEFINE_SPINLOCK(name)	spinlock_t (name)
#define TICKET_SHIFT	16

/* TODO: fix this if we add support for BE */
typedef struct {
	u16 owner;
	u16 next;
} spinlock_t __attribute__((aligned(4)));

static inline void spin_lock(spinlock_t *lock)
{
	unsigned int tmp;
	spinlock_t lockval, newval;

	asm volatile(
	/* Atomically increment the next ticket. */
"	prfm	pstl1strm, %3\n"
"1:	ldaxr	%w0, %3\n"
"	add	%w1, %w0, %w5\n"
"	stxr	%w2, %w1, %3\n"
"	cbnz	%w2, 1b\n"
	/* Did we get the lock? */
"	eor	%w1, %w0, %w0, ror #16\n"
"	cbz	%w1, 3f\n"
	/*
	 * No: spin on the owner. Send a local event to avoid missing an
	 * unlock before the exclusive load.
	 */
"	sevl\n"
"2:	wfe\n"
"	ldaxrh	%w2, %4\n"
"	eor	%w1, %w2, %w0, lsr #16\n"
"	cbnz	%w1, 2b\n"
	/* We got the lock. Critical section starts here. */
"3:"
	: "=&r" (lockval), "=&r" (newval), "=&r" (tmp), "+Q" (*lock)
	: "Q" (lock->owner), "I" (1 << TICKET_SHIFT)
	: "memory");
}

static inline void spin_unlock(spinlock_t *lock)
{
	asm volatile(
"	stlrh	%w1, %0\n"
	: "=Q" (lock->owner)
	: "r" (lock->owner + 1)
	: "memory");
}

#endif /* !_JAILHOUSE_ASM_SPINLOCK_H */

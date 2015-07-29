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
 *
 * Copied from arch/arm65/include/asm/spinlock.h in Linux
 */

#ifndef _JAILHOUSE_ASM_SPINLOCK_H
#define _JAILHOUSE_ASM_SPINLOCK_H

#define DEFINE_SPINLOCK(name)	spinlock_t (name)

/* TODO: fix this if we add support for BE */
typedef struct {
	u32 owner;
	u32 next;
} spinlock_t;

static inline void spin_lock(spinlock_t *lock)
{
	u32 owner, next, me, tmp;

	/* AARCH64_TODO: fix the prefetch with the right domain */
	asm volatile(
	/* Atomically increment the next ticket. */
"	prfm	pstl1strm, %4\n"
"1:	ldaxp	%w0, %w1, %4\n"
"	add	%w2, %w1, #1\n"
"	stxp	%w3, %w0, %w2, %4\n"
"	cbnz	%w3, 1b\n"
	/* Did we get the lock? */
"	eor	%w3, %w0, %w1\n"
"	cbz	%w3, 3f\n"
	/*
	 * No: spin on the owner. Send a local event to avoid missing an
	 * unlock before the exclusive load.
	 */
"	sevl\n"
"2:	wfe\n"
"	ldaxr	%w0, %5\n"
"	eor	%w3, %w0, %w1\n"
"	cbnz	%w3, 2b\n"
	/* We got the lock. Critical section starts here. */
"3:"
	: "=&r" (owner), "=&r" (me), "=&r" (next), "=&r" (tmp), "+Q" (*lock)
	: "Q" (lock->owner)	    // do we need Ump over here? ^
	: "memory");
}

static inline void spin_unlock(spinlock_t *lock)
{
	asm volatile(
"	stlr	%w1, %0\n"
	: "=Q" (lock->owner)
	: "r" (lock->owner + 1)
	: "memory");
}

#endif /* !_JAILHOUSE_ASM_SPINLOCK_H */

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

#include <asm/head.h>
#include <asm/processor.h>
#include <asm/sysregs.h>

.macro	ventry	label
	.align	7
	b	\label
.endm

.macro handle_vmexit exit_reason
	.align	7
	/* Fill the struct registers. Should comply with NUM_USR_REGS */
	stp	x29, x30, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9, x10, [sp, #-16]!
	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!

	mov	x1, #\exit_reason
	stp	x1, x0, [sp, #-16]!

	mrs	x0, tpidr_el2
	mov	x1, sp
	bl	arch_handle_exit
	b	vmreturn
.endm

	.text
	.globl hyp_vectors
	.align 11
hyp_vectors:
	ventry	.
	ventry	.
	ventry	.
	ventry	.

	handle_vmexit EXIT_REASON_EL2_ABORT
	handle_vmexit EXIT_REASON_EL2_IRQ
	ventry	.
	ventry	.

	handle_vmexit EXIT_REASON_EL1_ABORT
	ventry	.
	ventry	.
	ventry	.

	ventry	.
	ventry	.
	ventry	.
	ventry	.

.globl vmreturn
vmreturn:
	b	.

.globl hvc
hvc:
	hvc	#0
	ret

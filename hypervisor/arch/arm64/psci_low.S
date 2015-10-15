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
#include <asm/psci.h>

	.globl smc
	/*
	 * Since we trap all SMC instructions, it may be useful to forward them
	 * when it isn't a PSCI call. The shutdown code will also have to issue
	 * a real PSCI_OFF call on secondary CPUs.
	 */
smc:
	smc	#0
	ret

	.global _psci_cpu_off
	/* x0: struct psci_mbox* */
_psci_cpu_off:
	ldr	x2, =PSCI_INVALID_ADDRESS
	/* Clear mbox */
	str	x2, [x0]
	/*
	 * No reordering against the ldr below for the PEs in our domain, so no
	 * need for a barrier. Other CPUs will wait for an invalid address
	 * before issuing a CPU_ON.
	 */

	/* Wait for a CPU_ON call that updates the mbox */
1:	wfe
	ldr	x1, [x0]
	cmp	x1, x2
	beq	1b

	/* Jump to the requested entry, with a parameter */
	ldr	x2, [x0, #8]
	br	x1

	.global _psci_cpu_on
	/* x0: struct psci_mbox*, x1: entry, x2: context */
_psci_cpu_on:
	ldr	x3, =PSCI_INVALID_ADDRESS

	ldaxp	x4, x5, [x0]
	cmp	x4, x3
	bne	store_failed
	stxp	w4, x1, x2, [x0]
	cmp	w4, #0
	bne	store_failed

	mov	x0, #0
	ret

store_failed:
	clrex
	mov	x0, #PSCI_ALREADY_ON
	ret

	.global _psci_suspend_return
_psci_suspend_return:
	ret

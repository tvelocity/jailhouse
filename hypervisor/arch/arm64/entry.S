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
#include <asm/percpu.h>
#include <asm/platform.h>

/* Entry point for Linux loader module on JAILHOUSE_ENABLE */
	.text
	.globl arch_entry
arch_entry:
	/* x0: cpuid */
	mov	x18, x0		/* keep cpuid. we need it */

	/* AARCH64_TODO: save stub vector for later */

	/* get physical address of bootstrap_vectors */
	ldr	x0, =bootstrap_vectors
	hvc	#0	/* linux stub installs bootstrap_vectors */
	hvc	#0	/* bootstrap vector enters EL2 */

	/* the bootstrap vector returns us here in physical addressing */
el2_entry:
	mrs	x1, esr_el2
	lsr	x1, x1, #26
	cmp	x1, #0x16
	b.ne	.		/* not hvc */

	ldr	x1, =__page_pool
	mov	x2, #(1 << PERCPU_SIZE_SHIFT)
	/*
	 * percpu data = pool + cpuid * shift
	 * AARCH64_TODO: handle affinities
	 */
	madd	x1, x2, x18, x1
	msr	tpidr_el2, x1

	/* set SP for hypervisor */
	add	x2, x1, #PERCPU_STACK_END
	mov	sp, x2

	stp	x19, x20, [sp, #-16]!	/* push callee saved registers */
	stp	x21, x22, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x29, x30, [sp, #-16]!

	/* setup frame pointer */
	stp	xzr, xzr, [sp, #-16]!
	sub	x29, x2, #16

	/* install jailhouse vectors */
	ldr	x2, =hyp_vectors
	msr	vbar_el2, x2

	/* enable temporary mmu mappigns for early initialization */
	ldr	x0, =bootstrap_pt_l1
	bl	enable_mmu_el2

	mov	x0, x18

	/* Call entry(cpuid, struct per_cpu*) */
	bl	entry

	/* AARCH64_TODO: proper error / vmreturn handling */
	/*
	 * AARCH64_TODO: disable MMU for jailhouse disable
	 * x1: unsigned long vectors
	 */

	/* disable the MMU for EL2 */
	mrs	x9, sctlr_el2
	ldr	x10, =(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	bic	x9, x9, x10
	msr	sctlr_el2, x9
	isb

	msr	mair_el2, xzr
	msr	ttbr0_el2, xzr
	msr	tcr_el2, xzr
	isb

	/* restore vectors */
	msr	vbar_el2, x1

	eret

	.globl arch_cpu_activate_vmm
arch_cpu_activate_vmm:
	b	.

	.globl enable_mmu_el2
enable_mmu_el2:
	/*
	 * x0: u64 ttbr0_el2
	 * preserves x1 for arch_entry
	 */

	/* setup the MMU for EL2 hypervisor mappings */
	ldr	x2, =DEFAULT_MAIR_EL2
	msr	mair_el2, x2

	/* AARCH64_TODO: look into inner and outer shareable domains on our
	 * target, and how to handle properly here */
	ldr	x2, =(T0SZ | (TCR_RGN_WB_WA << TCR_IRGN0_SHIFT)		\
			   | (TCR_RGN_WB_WA << TCR_ORGN0_SHIFT)		\
			   | (TCR_INNER_SHAREABLE << TCR_SH0_SHIFT)	\
			   | (TCR_PS_40B << TCR_PS_SHIFT)		\
			   | TCR_EL2_RES1)
	msr	tcr_el2, x2

	msr	ttbr0_el2, x0

	tlbi	alle2
	dsb	nsh

	/* AARCH64_TODO: proper cache support */
	ldr	x2, =(SCTLR_M_BIT | SCTLR_EL2_RES1)
	msr	sctlr_el2, x2
	isb

	ret

/*
 * Using two pages, we can economically identity map the whole address space
 * of the machine (that is accessible with mappings starting from L1 with a 4KB
 * translation granule), with the 2MB block that includes the UART marked as
 * device memory. This allows us to start initializing the hypervisor before
 * we set up the final EL2 page tables.
 */
.align 12
bootstrap_pt_l1:
	addr = 0
	blk_sz = 1 << 30
        .rept 512
	.if (addr ^ UART_BASE) >> 30
        .quad addr | PAGE_DEFAULT_FLAGS
	.else
        .quad bootstrap_pt_l2 + PTE_TABLE_FLAGS
	.endif
	addr = addr + blk_sz
        .endr
bootstrap_pt_l2:
	addr = UART_BASE & ~((1 << 30) - 1)
	blk_sz = 1 << 21
        .rept 512
	.if (addr ^ UART_BASE) >> 21
        .quad addr | PAGE_DEFAULT_FLAGS
	.else
        .quad addr | PAGE_DEFAULT_FLAGS | PAGE_FLAG_DEVICE
	.endif
	addr = addr + blk_sz
        .endr

.macro	ventry	label
	.align	7
	b	\label
.endm

	.globl bootstrap_vectors
	.align 11
bootstrap_vectors:
	ventry	.
	ventry	.
	ventry	.
	ventry	.

	ventry	.
	ventry	.
	ventry	.
	ventry	.

	ventry	el2_entry
	ventry	.
	ventry	.
	ventry	.

	ventry	.
	ventry	.
	ventry	.
	ventry	.

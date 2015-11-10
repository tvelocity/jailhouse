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
#include <asm/jailhouse_hypercall.h>

/* Entry point for Linux loader module on JAILHOUSE_ENABLE */
	.text
	.globl arch_entry
arch_entry:
	/*
	 * x0: cpuid
	 *
	 * We don't have access to our own address space yet, so we will
	 * abuse some caller saved registers to preserve accross calls:
	 * x16: saved hyp vectors
	 * x17: cpuid
	 * x18: caller lr
	 */
	mov	x17, x0
	mov	x18, x30

	/* Note 1: After turning MMU off the CPU can start bypassing caches.
	 * But cached before data is kept in caches either until the CPU turns
	 * MMU on again or other coherent agents move cached data out. That's
	 * why there is no need to clean D-cache before turning MMU off.
	 *
	 * Note 2: We don't have to clean D-cache to protect against malicious
	 * guests, which can execute 'dc isw' (data or unified Cache line
	 * Invalidate by Set/Way) because when virtualization is enabled
	 * (HCR_EL2.VM == 1) then HW automatically upgrade 'dc isw' to
	 * 'dc cisw' (Clean + Invallidate). Executing Clean operation before
	 * Invalidate is safe in guests.
	 */

	/* keep the linux stub EL2 vectors for later */
	mov	x0, xzr
	hvc	#0
	mov	x16, x0

	/* install bootstrap_vectors */
	ldr	x0, =bootstrap_vectors
	hvc	#0
	hvc	#0	/* bootstrap vectors enter EL2 */

	/* the bootstrap vector returns us here in physical addressing */
el2_entry:
	mrs	x1, esr_el2
	lsr	x1, x1, #26
	cmp	x1, #0x16
	b.ne	.		/* not hvc */

	/* install jailhouse vectors */
	ldr	x1, =hyp_vectors
	msr	vbar_el2, x1

	/* enable temporary mmu mappigns for early initialization */
	ldr	x0, =bootstrap_pt_l0
	bl	enable_mmu_el2

	mov	x0, x17		/* preserved cpuid, will be passed to entry */
	ldr	x1, =__page_pool
	mov	x2, #(1 << PERCPU_SIZE_SHIFT)
	/*
	 * percpu data = pool + cpuid * shift
	 * AARCH64_TODO: handle affinities
	 */
	madd	x1, x2, x0, x1
	msr	tpidr_el2, x1

	/* set up the stack and push the root cell's callee saved registers */
	add	sp, x1, #PERCPU_STACK_END
	stp	x29, x18, [sp, #-16]!	/* note: our caller lr is in x18 */
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	/*
	 * We pad the stack, so we can consistently access the guest
	 * registers from either the initialization, or the exception
	 * handling code paths. 19 caller saved registers plus the
	 * exit_reason, which we don't use on entry.
	 */
	sub	sp, sp, 20 * 8

	mov	x29, xzr

	/* save the Linux stub vectors we kept earlier */
	add	x2, x1, #PERCPU_LINUX_SAVED_VECTORS
	str	x16, [x2]

	/* Call entry(cpuid, struct per_cpu*). Should not return. */
	bl	entry
	b	.

	.globl arch_shutdown_mmu
arch_shutdown_mmu:
	/* x0: struct percpu* */
	mov	x19, x0

	/* Note: no memory accesses must be done after turning MMU off. There
	 * is non-zero probability that cached data can be not syncronized with
	 * system memory. CPU can access data bypassing D-cache when MMU is off.
	 */

	/* hand over control of EL2 back to Linux */
	add	x1, x19, #PERCPU_LINUX_SAVED_VECTORS
	ldr	x2, [x1]
	msr	vbar_el2, x2

	/* disable the hypervisor MMU */
	mrs	x1, sctlr_el2
	ldr	x2, =(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	bic	x1, x1, x2
	msr	sctlr_el2, x1
	isb

	msr	mair_el2, xzr
	msr	ttbr0_el2, xzr
	msr	tcr_el2, xzr
	isb

	msr	tpidr_el2, xzr

	/* Call vmreturn(guest_registers) */
	add	x0, x19, #(PERCPU_STACK_END - 32 * 8)
	b	vmreturn

	.globl enable_mmu_el2
enable_mmu_el2:
	/*
	 * x0: u64 ttbr0_el2
	 */

	/* setup the MMU for EL2 hypervisor mappings */
	ldr	x1, =DEFAULT_MAIR_EL2
	msr	mair_el2, x1

	/* AARCH64_TODO: ARM architecture supports CPU clusters which could be
	 * in separate inner shareable domains. At the same time: "The Inner
	 * Shareable domain is expected to be the set of PEs controlled by
	 * a single hypervisor or operating system." (see p. 93 of ARM ARM)
	 * We should think what hw configuration we support by one instance of
	 * the hypervisor and choose Inner or Outter sharable domain.
	 */
	ldr	x1, =(T0SZ(48) | (TCR_RGN_WB_WA << TCR_IRGN0_SHIFT)	\
			       | (TCR_RGN_WB_WA << TCR_ORGN0_SHIFT)	\
			       | (TCR_INNER_SHAREABLE << TCR_SH0_SHIFT)\
			       | (TCR_PS_48B << TCR_PS_SHIFT)		\
			       | TCR_EL2_RES1)
	msr	tcr_el2, x1

	msr	ttbr0_el2, x0

	tlbi	alle2
	dsb	nsh

	/* Enable MMU, allow cacheability for instructions and data */
	ldr	x1, =(SCTLR_I_BIT | SCTLR_C_BIT | SCTLR_M_BIT | SCTLR_EL2_RES1)
	msr	sctlr_el2, x1
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
bootstrap_pt_l0:
	addr = 0
	blk_sz = 1 << 39
        .rept 512
	.if (addr >> 39) == (UART_BASE >> 39)
        .quad bootstrap_pt_l1_uart + PTE_TABLE_FLAGS
	.else
	.if (addr >> 39) == (JAILHOUSE_BASE >> 39)
        .quad bootstrap_pt_l1 + PTE_TABLE_FLAGS
	.else
        .quad 0
	.endif
	.endif
	addr = addr + blk_sz
        .endr
bootstrap_pt_l1:
#if (JAILHOUSE_BASE >> 39) != (UART_BASE)
	addr = JAILHOUSE_BASE & ~((1 << 39) - 1)
	blk_sz = 1 << 30
        .rept 512
        .quad addr | PAGE_DEFAULT_FLAGS
	addr = addr + blk_sz
        .endr
#endif
bootstrap_pt_l1_uart:
	addr = UART_BASE & ~((1 << 39) - 1)
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

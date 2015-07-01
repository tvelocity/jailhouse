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

#ifndef _JAILHOUSE_ASM_PROCESSOR_H
#define _JAILHOUSE_ASM_PROCESSOR_H

#include <jailhouse/types.h>
#include <jailhouse/utils.h>

#define SCTLR_M_BIT	(1 << 0)
#define SCTLR_A_BIT	(1 << 1)
#define SCTLR_C_BIT	(1 << 2)
#define SCTLR_CP15B_BIT (1 << 5)
#define SCTLR_ITD_BIT	(1 << 7)
#define SCTLR_SED_BIT	(1 << 8)
#define SCTLR_I_BIT	(1 << 12)
#define SCTLR_V_BIT	(1 << 13)
#define SCTLR_nTWI	(1 << 16)
#define SCTLR_nTWE	(1 << 18)
#define SCTLR_WXN_BIT	(1 << 19)
#define SCTLR_UWXN_BIT	(1 << 20)
#define SCTLR_FI_BIT	(1 << 21)
#define SCTLR_EE_BIT	(1 << 25)
#define SCTLR_TRE_BIT	(1 << 28)
#define SCTLR_AFE_BIT	(1 << 29)
#define SCTLR_TE_BIT	(1 << 30)

#define SCTLR_EL2_RES1	((2 << 4) | (1 << 11) | (1 << 16) | (1 << 18)	\
			| (2 << 22) | (2 << 28))

/* exception class */
#define ESR_EC_SHIFT		26
#define ESR_EC(hsr)		((hsr) >> ESR_EC_SHIFT & 0x3f)
/* instruction length */
#define ESR_IL_SHIFT		25
#define ESR_IL(hsr)		((hsr) >> ESR_IL_SHIFT & 0x1)
/* Instruction specific */
#define ESR_ICC_MASK		0x1ffffff
#define ESR_ICC(hsr)		((hsr) & ESR_ICC_MASK)
/* Exception classes values */
#define ESR_EC_UNK		0x00
#define ESR_EC_WFI		0x01
#define ESR_EC_CP15_32		0x03
#define ESR_EC_CP15_64		0x04
#define ESR_EC_CP14_32		0x05
#define ESR_EC_CP14_LC		0x06
#define ESR_EC_HCPTR		0x07
#define ESR_EC_CP10		0x08
#define ESR_EC_CP14_64		0x0c
#define ESR_EC_SVC_HYP		0x11
#define ESR_EC_HVC		0x12
#define ESR_EC_SMC		0x13
#define ESR_EC_IABT		0x20
#define ESR_EC_IABT_HYP		0x21
#define ESR_EC_PCALIGN		0x22
#define ESR_EC_DABT		0x24
#define ESR_EC_DABT_HYP		0x25
/* Condition code */
#define ESR_ICC_CV_BIT		(1 << 24)
#define ESR_ICC_COND(icc)	((icc) >> 20 & 0xf)

#define EXIT_REASON_EL2_ABORT	0x0
#define EXIT_REASON_EL2_IRQ	0x1
#define EXIT_REASON_EL1_ABORT	0x2

#define NUM_USR_REGS		31

#ifndef __ASSEMBLY__

struct registers {
	unsigned long exit_reason;
	/* r0 - r12 and lr. The other registers are banked. */
	unsigned long usr[NUM_USR_REGS];
};

#define dmb(domain)	asm volatile("dmb " #domain "\n" ::: "memory")
#define dsb(domain)	asm volatile("dsb " #domain "\n" ::: "memory")
#define isb()		asm volatile("isb\n")

#define wfe()		asm volatile("wfe\n")
#define wfi()		asm volatile("wfi\n")
#define sev()		asm volatile("sev\n")

static inline void cpu_relax(void)
{
	asm volatile("" : : : "memory");
}

static inline void memory_barrier(void)
{
	dmb(ish);
}

static inline void memory_load_barrier(void)
{
}

#define tlb_flush_guest()	asm volatile("tlbi vmalls12e1\n")

#endif /* !__ASSEMBLY__ */

#endif /* !_JAILHOUSE_ASM_PROCESSOR_H */

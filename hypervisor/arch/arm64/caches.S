/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 *
 * Authors:
 *  Dmitry Voytik <dmitry.voytik@huawei.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

/*  Note: interrupts are disabled when these functions are being called
 *
 * ARM ARM: "A DSB intended to ensure the completion of cache maintenance
 * operations must have an access type of both loads and stores." */

	.global arch_cpu_icache_flush
arch_cpu_icache_flush:
	b	.

/* Paramaters:
 *      x0: 0 - clean
 *          1 - clean + invalidate
 * Clobbers x1 - x13 registers
 */
	.global arch_cpu_dcaches_flush
arch_cpu_dcaches_flush:
	dsb	sy			/* sync buffered writes before start */
	mov	w1, #1
	mrs	x6, CLIDR_EL1
	and	w3, w6, #0x07000000
	lsr	w3, w3, #23		/* Level of Coherency */
	cbz	w3, done
	mov	w10, #0			/* cur_cache_level_x2 = 0 */
next_level:
	add	w2, w10, w10, LSR #1	/* current cache level x 3 */
	lsr	w8, w6, w2		/* Ctype - cache type for this level */
	and	w8, w8, #0x7
	cmp	w8, #2
	b.lt	skip_level		/* none or only icache at this level */
	msr	CSSELR_EL1, x10		/* select this cache level */
	isb				/* flush CPU pipeline so CCSIDR_EL1 */
	mrs	x8, CCSIDR_EL1		/* is up to date here */
	and	w2, w8, #0x7		/* w2 = log2(bytes_in_cache_line) -4 */
	add	w2, w2, #4
	ubfx	w4, w8, #3, #10		/* right aligned max way number */
	clz	w5, w4			/* w5 = 32-log2(ways), bit position of
					   way in dc operand */
	lsl	w9, w4, w5		/* w9 = max way number, aligned to
					   position in dc operand */
	lsl	w12, w1, w5		/* decrement way number per iteration */
	lsl	w13, w1, w2		/* 1 << log2(bytes_in_cache_line) */
next_way:
	ubfx	w7, w8, #13, #15	/* number of sets in current level */
	lsl	w7, w7, w2		/* align to position for dc operand */
next_set:
	orr	w11, w10, w9		/* cache number and way number */
	orr	w11, w11, w7		/* and set number for dc instruction */
	cbnz	x0, clean_invalidate	/* if x0 != 0 then clean + invalidate */
	dc	csw, x11		/* data cache clean */
	b	futher
clean_invalidate:
	dc	cisw, x11		/* data cache clean + invalidate */
futher:
	subs	w7, w7, w13		/* decrement set number */
	b.ge	next_set
	subs	x9, x9, x12		/* decrement way number */
	b.ge	next_way
skip_level:
	add	w10, w10, #2		/* cur_cache_level_x2 += 2 */
	cmp	w3, w10
	b.gt	next_level
done:
	dsb	sy			/* block until dc instr completes */
	isb
	ret

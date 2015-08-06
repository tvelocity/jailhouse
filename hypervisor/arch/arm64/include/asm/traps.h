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

#ifndef _JAILHOUSE_ASM_TRAPS_H
#define _JAILHOUSE_ASM_TRAPS_H

#ifndef __ASSEMBLY__

enum trap_return {
	TRAP_HANDLED		= 1,
	TRAP_UNHANDLED		= 0,
	TRAP_FORBIDDEN		= -1,
};

struct trap_context {
};

void access_cell_reg(struct trap_context *ctx, u8 reg, unsigned long *val,
		     bool is_read);
void arch_skip_instruction(struct trap_context *ctx);

#endif /* !__ASSEMBLY__ */
#endif /* !_JAILHOUSE_ASM_TRAPS_H */

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

#ifndef _JAILHOUSE_ASM_SETUP_H
#define _JAILHOUSE_ASM_SETUP_H

#include <asm/head.h>
#include <asm/percpu.h>

#ifndef __ASSEMBLY__

#include <jailhouse/string.h>

void enable_mmu_el2(page_table_t ttbr0_el2);

int arch_map_device(void *paddr, void *vaddr, unsigned long size);
int arch_unmap_device(void *addr, unsigned long size);

#endif /* !__ASSEMBLY__ */
#endif /* !_JAILHOUSE_ASM_SETUP_H */

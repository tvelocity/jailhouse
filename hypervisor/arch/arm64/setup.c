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

#include <jailhouse/entry.h>

int arch_init_early(void)
{
	return -EINVAL;
}

int arch_cpu_init(struct per_cpu *cpu_data)
{
	return -EINVAL;
}

int arch_init_late(void)
{
	return -EINVAL;
}

void __attribute__((noreturn)) arch_cpu_activate_vmm(struct per_cpu *cpu_data)
{
	while(1);
}

void arch_cpu_restore(struct per_cpu *cpu_data, int return_code)
{
}

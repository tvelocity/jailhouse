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

#include <jailhouse/control.h>

/*
 * Stubs used by arch independent code
 */

int arch_cell_create(struct cell *cell)
{
	return -EINVAL;
}

void arch_flush_cell_vcpu_caches(struct cell *cell)
{
}

void arch_cell_destroy(struct cell *cell)
{
}

void arch_config_commit(struct cell *cell_added_removed)
{
}

void arch_shutdown(void)
{
}

void arch_suspend_cpu(unsigned int cpu_id)
{
}

void arch_resume_cpu(unsigned int cpu_id)
{
}

void arch_reset_cpu(unsigned int cpu_id)
{
}

void arch_park_cpu(unsigned int cpu_id)
{
}

void arch_shutdown_cpu(unsigned int cpu_id)
{
}

void __attribute__((noreturn)) arch_panic_stop(void)
{
	while(1);
}

void arch_panic_park(void)
{
}

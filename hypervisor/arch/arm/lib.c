/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) ARM Limited, 2014
 *
 * Authors:
 *  Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <jailhouse/processor.h>
#include <jailhouse/string.h>
#include <jailhouse/types.h>
#include <asm/percpu.h>
#include <asm/sysregs.h>

int phys_processor_id(void)
{
	return this_cpu_data()->cpu_id;
}

void *memcpy(void *dest, const void *src, unsigned long n)
{
	unsigned long i;
	const char *csrc = src;
	char *cdest = dest;

	for (i = 0; i < n; i++)
		cdest[i] = csrc[i];

	return dest;
}

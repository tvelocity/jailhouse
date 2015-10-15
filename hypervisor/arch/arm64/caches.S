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

	.global arch_cpu_dcaches_flush
arch_cpu_dcaches_flush:
	b	.

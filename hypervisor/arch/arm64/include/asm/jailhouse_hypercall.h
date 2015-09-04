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

#define JAILHOUSE_BASE			0xfc000000
#define JAILHOUSE_IOMAP_ADDR		0xffffff8c00000000
#define JAILHOUSE_IDENT_MAPPED		1

/* CPU statistics */
#define JAILHOUSE_CPU_STAT_VMEXITS_MAINTENANCE	JAILHOUSE_GENERIC_CPU_STATS
#define JAILHOUSE_CPU_STAT_VMEXITS_VIRQ		JAILHOUSE_GENERIC_CPU_STATS + 1
#define JAILHOUSE_CPU_STAT_VMEXITS_VSGI		JAILHOUSE_GENERIC_CPU_STATS + 2
#define JAILHOUSE_NUM_CPU_STATS			JAILHOUSE_GENERIC_CPU_STATS + 3

#ifndef __ASSEMBLY__

struct jailhouse_comm_region {
	COMM_REGION_GENERIC_HEADER;
};

static inline __u32 jailhouse_call(__u32 num)
{
	while(1);
	return 0;
}

static inline __u32 jailhouse_call_arg1(__u32 num, __u32 arg1)
{
	while(1);
	return 0;
}

static inline __u32 jailhouse_call_arg2(__u32 num, __u32 arg1, __u32 arg2)
{
	while(1);
	return 0;
}

static inline void
jailhouse_send_msg_to_cell(struct jailhouse_comm_region *comm_region,
			   __u32 msg)
{
	while(1);
}

static inline void
jailhouse_send_reply_from_cell(struct jailhouse_comm_region *comm_region,
			       __u32 reply)
{
	while(1);
}

#endif /* !__ASSEMBLY__ */

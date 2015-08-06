#
# Jailhouse AArch64 support
#
# Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
#
# Authors:
#  Antonios Motakis <antonios.motakis@huawei.com>
#
# This work is licensed under the terms of the GNU GPL, version 2.  See
# the COPYING file in the top-level directory.
#

include $(CONFIG_MK)

KBUILD_AFLAGS := $(filter-out -include asm/unified.h,$(KBUILD_AFLAGS))

always := built-in.o

obj-y += setup.o control.o lib.o mmio.o traps.o string.o
obj-y += entry.o caches.o exception.o
obj-y += ../arm/mmu_cell.o ../arm/paging.o
obj-y += ../arm/dbg-write.o
obj-y += ../arm/irqchip.o ../arm/gic-common.o

obj-$(CONFIG_ARM_GIC) += ../arm/gic-v2.o
obj-$(CONFIG_SERIAL_AMBA_PL011) += ../arm/dbg-write-pl011.o

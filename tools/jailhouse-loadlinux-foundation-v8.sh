#!/bin/sh

# Note: this hacky script is a temporary solution and the functionality will
# be moved to ./tools/jailhouse-cell-linux
#
# Note: put linux-loader.bin, kernel-inmate-foundation-v8.dtb, nonroot_Image
# in the /root directory.

DTB_ADDR="   0x00 0x00 0xe0 0x0f 0x00 0x00 0x00 0x00"
KERNEL_ADDR="0x00 0x00 0x28 0x00 0x00 0x00 0x00 0x00"

R=/root
echo -ne "$(printf '\\x%x' $DTB_ADDR)" > linux_cfg
echo -ne "$(printf '\\x%x' $KERNEL_ADDR)" >> linux_cfg
hexdump -C $R/linux_cfg

jailhouse cell load --name linux-inmate-demo \
	$R/linux-loader.bin -a 0x0 \
	$R/linux_cfg -a 0x4000 \
	$R/kernel-inmate-foundation-v8.dtb -a 0xfe00000 \
	$R/nonroot_Image -a 0x280000 \

jailhouse cell start --name linux-inmate-demo

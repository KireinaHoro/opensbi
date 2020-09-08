#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Pengcheng Xu <i@jsteward.moe>
#

# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =

# Blobs to build
FW_TEXT_START=0x80000000
FW_DYNAMIC=n
FW_JUMP=n
FW_PAYLOAD=y
#FW_PAYLOAD_PATH=$(platform_build_dir)/firmware/payloads/u-boot.bin
#FW_PAYLOAD_PATH=$(kernel_bin)
# outside PMP0 zone
FW_PAYLOAD_OFFSET=0x200000
FW_PAYLOAD_FDT=zcu102.dtb
FW_OPTIONS=0x1
PLATFORM_RISCV_ABI=lp64d

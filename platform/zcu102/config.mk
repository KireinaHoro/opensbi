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
FW_TEXT_START=0x800000000
# TVM payload will be loaded at 1M offset
FW_TVM_PAYLOAD_OFFSET=0x100000
FW_DYNAMIC=n
FW_JUMP=n
FW_PAYLOAD=y
#FW_PAYLOAD_PATH=$(platform_build_dir)/firmware/payloads/u-boot.bin
# outside PMP0 zone
FW_PAYLOAD_OFFSET=0x20000
FW_PAYLOAD_FDT=zcu102.dtb

PLATFORM_RISCV_ISA=rv64imac
PLATFORM_RISCV_ABI=lp64

# SBI_SCRATCH_NO_BOOT_PRINTS
FW_OPTIONS=0x1

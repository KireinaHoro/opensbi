#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Pengcheng Xu <i@jsteward.moe>
#

platform-objs-y += platform.o
platform-dtb-y += zcu102.dtb
ifdef ZCU102_ENABLED_HART_MASK
platform-genflags-y += -DZCU102_ENABLED_HART_MASK=$(ZCU102_ENABLED_HART_MASK)
endif

#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Pengcheng Xu <i@jsteward.moe>
#

platform-objs-y += platform.o
ifdef EDGEBOARD_ENABLED_HART_MASK
platform-genflags-y += -DEDGEBOARD_ENABLED_HART_MASK=$(EDGEBOARD_ENABLED_HART_MASK)
endif

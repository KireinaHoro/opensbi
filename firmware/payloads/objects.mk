#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

firmware-bins-$(FW_PAYLOAD) += payloads/test.bin
#firmware-bins-$(FW_PAYLOAD) += payloads/u-boot.bin

test-y += test_head.o
test-y += test_main.o
test-y += support.o
test-y += nvdla.o
test-y += sdp.o

%/test.o: $(foreach obj,$(test-y),%/$(obj))
	$(call merge_objs,$@,$^)

%/test.dep: $(foreach dep,$(test-y:.o=.dep),%/$(dep))
	$(call merge_deps,$@,$^)

#u-boot_root=$(src_dir)/../u-boot
#u-boot_bin=$(platform_build_dir)/firmware/payloads/u-boot.bin
#
#.PHONY: $(u-boot_bin)
#$(u-boot_bin):
#	$(MAKE) -C $(u-boot_root)
#	cp $(u-boot_root)/u-boot.bin $@

kernel_root=$(src_dir)/../../linux
kernel_bin=$(src_dir)/firmware/payloads/Image

sel4_bin=$(platform_build_dir)/firmware/payloads/elfloader.bin
elfloader_elf=$(src_dir)/firmware/payloads/elfloader
$(sel4_bin): $(elfloader_elf)
	$(OBJCOPY) -S -O binary $< $@

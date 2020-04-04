/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)

#define DDR_BASE 0x800000000
#define DDR_SIZE 0x80000000

SECTIONS
{
#ifdef FW_PAYLOAD_OFFSET
	. = FW_TEXT_START + FW_PAYLOAD_OFFSET;
#else
	. = ALIGN(FW_PAYLOAD_ALIGN);
#endif

	PROVIDE(_payload_start = .);

	. = ALIGN(0x1000); /* Need this to create proper sections */

	/* Beginning of the code section */

	.text :
	{
		PROVIDE(_text_start = .);
		*(.entry)
		*(.text)
		*(.text.*)
		. = ALIGN(8);
		PROVIDE(_text_end = .);
	}

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	/* End of the code sections */

	/* Beginning of the read-only data sections */

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	.rodata :
	{
		PROVIDE(_rodata_start = .);
		*(.rodata .rodata.*)
		. = ALIGN(8);
		PROVIDE(_rodata_end = .);
	}

	/* End of the read-only data sections */

	/* Beginning of the read-write data sections */

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	.data :
	{
		PROVIDE(_data_start = .);

		*(.data)
		*(.data.*)
		*(.readmostly.data)
		*(*.data)
		. = ALIGN(8);

		PROVIDE(_data_end = .);
	}

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	.bss :
	{
		PROVIDE(_bss_start = .);
		*(.bss)
		*(.bss.*)
		. = ALIGN(8);
		PROVIDE(_bss_end = .);
	}

	.sbss :
	{
		PROVIDE(_sbss_start = .);
		*(.sbss)
		*(.sbss.*)
		. = ALIGN(8);
		PROVIDE(_sbss_end = .);
	}

	/* End of the read-write data sections */

	/* 128KiB heap, page aligned, for monitor */
	. = ALIGN(0x1000);
	PROVIDE(_heap_start = .);

	. += 0x20000;
	PROVIDE(_heap_end = .);

	/* TVM generated code starts here */
	. = FW_TEXT_START + FW_TVM_PAYLOAD_OFFSET;
	PROVIDE(_tvm_start = .);

	/* trap and normal stack */
	. = DDR_BASE + DDR_SIZE - 0x11000;
	PROVIDE(_stack_end = .);

	. += 0x1000;
	PROVIDE(_trap_stack = .);

	. = DDR_BASE + DDR_SIZE;
	PROVIDE(_stack_top = .);
}

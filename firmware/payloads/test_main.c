/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <malloc.h>

#include "common.h"
#include "routines.h"

#define MAXLINE 64

int test_main(unsigned long a0, unsigned long a1)
{
	char buf[MAXLINE];
	printf(INIT_MAGIC);

	// read two integers as execute
	sbi_getline(buf, MAXLINE - 1);

	uint64_t func_addr, stop_addr;
	sscanf(buf, "%ld%ld", &func_addr, &stop_addr);

	// clear the stop_addr to write unimp (2 bytes, assuming C extension)
	// the trap handler will write sepc and EXIT_MAGIC back to ARM
	memset((void*)stop_addr, 0, 2);

	// jump to the loaded code
	void (*func)() = (void(*)())func_addr;
	func();

	// we shouldn't reach here; jump to the handler
	__asm__ volatile("unimp");
	
	return 0;
}

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

int test_main(unsigned long a0, unsigned long a1)
{
	printf("\nS-mode entry point\n");

	uint8_t *a = malloc(4096);
	printf("malloc returned %p\n", a);

	uint8_t *b = memalign(0x1000, 0x1000);
	printf("memalign returned %p\n", b);

	printf("\nTest printf again\n");

	return 0;

	matmul_test();
}

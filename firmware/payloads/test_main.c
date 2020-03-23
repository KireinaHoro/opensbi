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

	vminit();

	printf("\nTest printf again\n");

	return 0;

	matmul_test();
}

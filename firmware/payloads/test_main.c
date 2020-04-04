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

	while (true) {
		sbi_getline(buf, MAXLINE - 1);
		printf("%s\n", buf);
	}

	return 0;
}

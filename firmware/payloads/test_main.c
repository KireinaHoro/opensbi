/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include "common.h"

int irqs[] = {NVDLA_IRQ};

// a0 holds the hartid
// a1 holds the device tree address
void test_main(unsigned long a0, unsigned long a1) {
    // store cpuid
    w_tp(a0);
    printf("S-mode entry\n");

    enable_irq(irqs, ELEMS(irqs));

    printf("Turning on interrupts...\n");
    intr_on();

    enable_dla();

    printf("Waiting for interrupt...\n");

    while (true) {
        wfi();
    }

    exit(0);
}

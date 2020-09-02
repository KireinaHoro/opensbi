/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include "common.h"

#include <stdio.h>
#include <stdlib.h>

#define FREQ (100 * 1000 * 1000)
#define DELAY_SECONDS 3
#define DELAY_TICKS (DELAY_SECONDS * FREQ)

void delay() {
    for (int i = 0; i < 100 * 1000 * 1000; ++i) {
        asm volatile("nop");
    }
}

void print_pwm_counters() {
    int count = *(uint32_t *)(PWM_COUNT)&0x7fffffff;
    int pwms = *(uint32_t *)(PWM_PWMS)&0xffff;
    printf("pwmcount=%d pwms=%d\n", count, pwms);
}

void handle_pwm_irq() {
    printf("PWM Timer interrupt - firing every %d seconds\n", DELAY_SECONDS);

    // clear counter
    *(uint32_t *)(PWM_COUNT) = 0;
    // clear interrupts
    *(uint32_t *)(PWM_CFG) &= ~(PWMCMP0IP | PWMCMP1IP | PWMCMP2IP | PWMCMP3IP);
}

static inline void sbi_ecall_console_puts(const char *str) {
    while (str && *str)
        sbi_ecall_console_putc(*str++);
}

static inline void sbi_ecall_console_printhex(uint64_t num) {
    sbi_ecall_console_puts("0x");
    for (int i = 15; i >= 0; --i) {
        int digit = (num & (0xfL << (4 * i))) >> (4 * i);
        if (digit > 9) {
            sbi_ecall_console_putc(digit - 10 + 'a');
        } else {
            sbi_ecall_console_putc(digit + '0');
        }
    }
}

uint64_t cpuid() { return r_tp(); }

// a0 holds the hartid
// a1 holds the device tree address
void test_main(unsigned long a0, unsigned long a1) {
    // store cpuid
    w_tp(a0);
    printf("Test payload running\n");

    vminit();


    exit(0);
}

void trap_handler() {
    uint64_t sepc = r_sepc();
    uint64_t scause = r_scause();

    bool intr = (scause & (1L << 63)) != 0;

    printf("\nTrap handler called: sepc=%#lx scause=%#lx stval=%#lx\n", sepc,
           scause, r_stval());

    if (!intr) {
        uint8_t opc = *(uint8_t *)(sepc)&0b11;
        bool rvc = opc != 0x3;
        // recover to next instruction
        printf("RVC: %s; correcting PC and continuing...\n",
               rvc ? "true" : "false");
        sepc += rvc ? 2 : 4;
        w_sepc(sepc);
    } else {
        // printf("Interrupt! sip=%#lx sie=%#lx\n", r_sip(), r_sie());

        if ((scause & 0xff) == 0x9) {
            // claim IRQ from PLIC
            int hart = cpuid();
            int irq = *(uint32_t *)PLIC_SCLAIM(hart);
            // printf("External interrupt IRQ %d\n", irq);

            if (irq == PWM0_IRQ) {
                handle_pwm_irq();
            } else {
                printf("Spurious IRQ\n");
            }

            // printf("Completing IRQ %d\n", irq);
            *(uint32_t *)PLIC_SCLAIM(hart) = irq;
        }
    }
    printf("Halting...\n");
    while (true) {
        wfi();
    }
}

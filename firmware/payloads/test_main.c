/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include "common.h"

#include <stdlib.h>
#include <stdio.h>

#define FREQ (100*1000*1000)
#define DELAY_SECONDS 3
#define DELAY_TICKS (DELAY_SECONDS * FREQ)

void delay() {
    for (int i = 0; i < 100*1000*1000; ++i) {
        asm volatile("nop");
    }
}

void print_pwm_counters() {
    int count = *(uint32_t*)(PWM_COUNT) & 0x7fffffff;
    int pwms = *(uint32_t*)(PWM_PWMS) & 0xffff;
    printf("pwmcount=%d pwms=%d\n", count, pwms);
}

void handle_pwm_irq() {
    printf("PWM Timer interrupt - firing every %d seconds\n", DELAY_SECONDS);

    // clear counter
    *(uint32_t*)(PWM_COUNT) = 0;
    // clear interrupts
    *(uint32_t*)(PWM_CFG) &= ~(PWMCMP0IP|PWMCMP1IP|PWMCMP2IP|PWMCMP3IP);
}

static inline void sbi_ecall_console_puts(const char *str)
{
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

uint64_t cpuid() {
    return r_tp();
}

// a0 holds the hartid
// a1 holds the device tree address
void test_main(unsigned long a0, unsigned long a1)
{
    // store cpuid
    w_tp(a0);
	printf("Test payload running\n");

    printf("Setting up PLIC...\n");
    *(uint32_t*)(PLIC + PWM0_IRQ * 4) = 1;

    printf("Enabling interrupt for PWM0...\n");
    uint64_t hart = cpuid();
    // enable PWM0
    *(uint32_t*)PLIC_SENABLE(hart) = (1 << PWM0_IRQ);
    // set S-mode priority threshold to 0
    *(uint32_t*)PLIC_SPRIORITY(hart) = 0;

    printf("Initializing PWM...\n");
    *(uint32_t*)(PWM_CMP0) = DELAY_TICKS >> 15; // ticks / 2^15
    *(uint32_t*)(PWM_CMP1) = 0xffff; // disabled
    *(uint32_t*)(PWM_CMP2) = 0xffff; // disabled
    *(uint32_t*)(PWM_CMP3) = 0xffff; // disabled
    *(uint32_t*)(PWM_CFG) = (15) | PWMZEROCMP | PWMSTICKY;
    *(uint32_t*)(PWM_COUNT) = 0;

    printf("Enabling PWM...\n");
    *(uint32_t*)(PWM_CFG) |= PWMENALWAYS;

    printf("Turning on interrupts...\n");
    intr_on();

    while (true) {
        print_pwm_counters();
        wfi();
    }

    exit(0);
}

void trap_handler() {
    uint64_t sepc = r_sepc();
    uint64_t scause = r_scause();

    bool intr = (scause & (1L << 63)) != 0;

    //printf("\nTrap handler called: sepc=%#lx scause=%#lx stval=%#lx\n", sepc, scause, r_stval());

    if (!intr) {
        uint8_t opc = *(uint8_t*)(sepc) & 0b11;
        bool rvc = opc != 0x3;
        // recover to next instruction
        printf("RVC: %s; correcting PC and continuing...\n", rvc ? "true" : "false");
        sepc += rvc ? 2 : 4;
        w_sepc(sepc);
    } else {
        //printf("Interrupt! sip=%#lx sie=%#lx\n", r_sip(), r_sie());

        if ((scause & 0xff) == 0x9) {
            // claim IRQ from PLIC
            int hart = cpuid();
            int irq = *(uint32_t*)PLIC_SCLAIM(hart);
            //printf("External interrupt IRQ %d\n", irq);

            if (irq == PWM0_IRQ) {
                handle_pwm_irq();
            } else {
                printf("Spurious IRQ\n");
            }

            //printf("Completing IRQ %d\n", irq);
            *(uint32_t*)PLIC_SCLAIM(hart) = irq;
        }
        
        //printf("Resuming...\n");
    }
}

int _write(int file, char *ptr, int len)
{
    int i;
    file = file;
    for (i = 0; i < len; i++) {
        sbi_ecall_console_putc(*ptr++);
    }
    return len;
}

void _exit(int n)
{
    sbi_ecall_console_puts("exit(");
    sbi_ecall_console_printhex(n);
    sbi_ecall_console_puts(") called, halting\n");
    while (true) {
        wfi();
    }
}

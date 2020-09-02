#pragma once

#include <sbi/sbi_ecall_interface.h>
#include <stdbool.h>
#include <stdint.h>

// SBI calls
#define SBI_ECALL(__num, __a0, __a1, __a2)                                     \
    ({                                                                         \
        register unsigned long a0 asm("a0") = (unsigned long)(__a0);           \
        register unsigned long a1 asm("a1") = (unsigned long)(__a1);           \
        register unsigned long a2 asm("a2") = (unsigned long)(__a2);           \
        register unsigned long a7 asm("a7") = (unsigned long)(__num);          \
        asm volatile("ecall"                                                   \
                     : "+r"(a0)                                                \
                     : "r"(a1), "r"(a2), "r"(a7)                               \
                     : "memory");                                              \
        a0;                                                                    \
    })
#define SBI_ECALL_0(__num) SBI_ECALL(__num, 0, 0, 0)
#define SBI_ECALL_1(__num, __a0) SBI_ECALL(__num, __a0, 0, 0)
#define SBI_ECALL_2(__num, __a0, __a1) SBI_ECALL(__num, __a0, __a1, 0)

#define sbi_ecall_console_putc(c) SBI_ECALL_1(SBI_EXT_0_1_CONSOLE_PUTCHAR, (c))

// WFI
#define wfi()                                                                  \
    do {                                                                       \
        __asm__ __volatile__("wfi" ::: "memory");                              \
    } while (0)

// CSRs
#include "riscv.h"

// PLIC
#define PLIC 0x0c000000L
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)

#include "mapping.h"

// IRQs
#define PWM0_IRQ 4
#define PWM1_IRQ 5
#define PWM2_IRQ 6
#define PWM3_IRQ 7

// PWM registers
#define PWM_BASE 0x10020000
#define PWM_CFG (PWM_BASE + 0x0)
#define PWM_COUNT (PWM_BASE + 0x8)
#define PWM_PWMS (PWM_BASE + 0x10)
#define PWM_CMP0 (PWM_BASE + 0x20)
#define PWM_CMP1 (PWM_BASE + 0x24)
#define PWM_CMP2 (PWM_BASE + 0x28)
#define PWM_CMP3 (PWM_BASE + 0x2c)

// pwmcfg fields
#define PWMSCALE 0b1111 // [3:0]
#define PWMSTICKY (1 << 8)
#define PWMZEROCMP (1 << 9)
#define PWMDEGLITCH (1 << 10)
#define PWMENALWAYS (1 << 12)
#define PWMENONESHOT (1 << 13)
#define PWMCMP0CENTER (1 << 16)
#define PWMCMP1CENTER (1 << 17)
#define PWMCMP2CENTER (1 << 18)
#define PWMCMP3CENTER (1 << 19)
#define PWMCMP0GANG (1 << 24)
#define PWMCMP1GANG (1 << 25)
#define PWMCMP2GANG (1 << 26)
#define PWMCMP3GANG (1 << 27)
#define PWMCMP0IP (1 << 28)
#define PWMCMP1IP (1 << 29)
#define PWMCMP2IP (1 << 30)
#define PWMCMP3IP (1 << 31)

// virtual memory
#define PTE_V (1L << 0) // valid
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4)
#define PTE_G (1L << 5)
#define PTE_A (1L << 6)
#define PTE_D (1L << 7)

#define PGSIZE 4096  // bytes per page
#define PGSHIFT 12   // bits of offset within a page
#define PXMASK 0x1FF // 9 bits
#define PA2PTE(pa) ((((uint64)pa) >> 12) << 10)
#define PTE2PA(pte) (((pte) >> 10) << 12)

#define SATP_SV39 (8L << 60)
#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64)pagetable) >> 12))

#define CLINT 0x2000000L
#define PLIC 0x0c000000L
#define UART 0x10010000L

typedef uint64_t uint64;
typedef uint64_t pte_t;
typedef uint64_t pde_t;
typedef uint64_t *pagetable_t; // 512 PTEs in a single page

void ram_bench();
void vminit();

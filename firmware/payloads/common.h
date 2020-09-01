#pragma once

#include <sbi/sbi_ecall_interface.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

#define ELEMS(arr) (sizeof(arr) / sizeof(arr[0]))

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

// IRQs
#define UART0_IRQ 1
#define UART1_IRQ 2
#define NVDLA_IRQ 3

void delay();
uint64_t cpuid();

typedef void (*irq_handler_t)();
void enable_irq(int *irqs, size_t count);
void hexdump(const void *data, size_t size);

#define TRACE(x)                                                               \
    { printf("%s(%s:%d) %s\n", __func__, __FILENAME__, __LINE__, (x)); }

#define DUMP_SIZE 128

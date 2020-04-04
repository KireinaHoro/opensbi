#include <sbi/sbi_ecall_interface.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>

#define SBI_ECALL(__num, __a0, __a1, __a2)                                    \
	({                                                                    \
		register unsigned long a0 asm("a0") = (unsigned long)(__a0);  \
		register unsigned long a1 asm("a1") = (unsigned long)(__a1);  \
		register unsigned long a2 asm("a2") = (unsigned long)(__a2);  \
		register unsigned long a7 asm("a7") = (unsigned long)(__num); \
		asm volatile("ecall"                                          \
			     : "+r"(a0)                                       \
			     : "r"(a1), "r"(a2), "r"(a7)                      \
			     : "memory");                                     \
		a0;                                                           \
	})

#define SBI_ECALL_0(__num) SBI_ECALL(__num, 0, 0, 0)
#define SBI_ECALL_1(__num, __a0) SBI_ECALL(__num, __a0, 0, 0)
#define SBI_ECALL_2(__num, __a0, __a1) SBI_ECALL(__num, __a0, __a1, 0)

#define sbi_ecall_console_getc() SBI_ECALL_0(SBI_EXT_0_1_CONSOLE_GETCHAR)
#define sbi_ecall_console_putc(c) SBI_ECALL_1(SBI_EXT_0_1_CONSOLE_PUTCHAR, (c))

void sbi_ecall_console_puts(const char *str);
char sbi_getc();
void sbi_getline(char *line, size_t len);

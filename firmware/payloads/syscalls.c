#include <sbi/sbi_ecall_interface.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "common.h"

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

void sbi_ecall_console_puts(const char *str)
{
	while (str && *str)
		sbi_ecall_console_putc(*str++);
}

static inline void sbi_ecall_console_printhex(uint64_t num)
{
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

static inline char sbi_getc()
{
	char c;
	while ((c = sbi_ecall_console_getc()) == 255)
		;
	return c;
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

extern intptr_t _heap_start;
extern intptr_t _stack_end;

intptr_t _sbrk(ptrdiff_t heap_incr)
{
	static intptr_t heap_end = (intptr_t)&_heap_start;

	intptr_t prev_heap_end;
	intptr_t new_heap_end;

	prev_heap_end = heap_end;
	new_heap_end  = prev_heap_end + heap_incr;

	if (new_heap_end >= (intptr_t)&_stack_end) {
		errno = ENOMEM;
		return -1;
	}

	heap_end = new_heap_end;

	return prev_heap_end;
}

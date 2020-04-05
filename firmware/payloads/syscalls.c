#include "routines.h"
#include "common.h"

void sbi_ecall_console_puts(const char *str)
{
	while (str && *str)
		sbi_ecall_console_putc(*str++);
}

void sbi_ecall_console_printhex(uint64_t num)
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

char sbi_getc()
{
	char c;
	while ((c = sbi_ecall_console_getc()) == 255);
	return c;
}

void sbi_getline(char *line, size_t len) {
	memset(line, 0, len);
	int i = 0;
	while (i < len - 1) {
		char new = (line[i++] = sbi_getc());
		if (new == '\n' || new == '\r') {
			return;
		}
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
	sbi_ecall_console_puts(EXIT_MAGIC);
	while (true) {
		wfi();
	}
}

extern intptr_t _heap_start;
extern intptr_t _heap_end;

intptr_t _sbrk(ptrdiff_t heap_incr)
{
	static intptr_t heap_end = (intptr_t)&_heap_start;

#ifdef DEBUG_SBRK
	sbi_ecall_console_puts("sbrk(");
	sbi_ecall_console_printhex(heap_incr);
	sbi_ecall_console_puts(") called, heap_end = ");
	sbi_ecall_console_printhex(heap_end);
	sbi_ecall_console_puts("\n");
#endif

	intptr_t prev_heap_end;
	intptr_t new_heap_end;

	prev_heap_end = heap_end;
	new_heap_end  = prev_heap_end + heap_incr;

	if (new_heap_end >= (intptr_t)&_heap_end) {
		errno = ENOMEM;
		return -1;
	}

	heap_end = new_heap_end;

	return prev_heap_end;
}

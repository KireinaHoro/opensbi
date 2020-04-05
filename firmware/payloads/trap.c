#include <stdbool.h>

#include "common.h"
#include "routines.h"

/* stvec needs to be aligned to 4-byte boundary */
__attribute__((optimize("align-functions=4"))) void trap()
{
	sbi_ecall_console_printhex(read_csr(sepc));
	sbi_ecall_console_puts(EXIT_MAGIC);
	while (true) {
		wfi();
	}
}

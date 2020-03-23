#include <stdbool.h>

#include "common.h"

/* stvec needs to be aligned to 4-byte boundary */
__attribute__((optimize("align-functions=4"))) void trap()
{
	sbi_ecall_console_puts("trap() called\n");
	while (true) {
		wfi();
	}
}

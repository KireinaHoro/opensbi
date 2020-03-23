#include <stdbool.h>

#include "common.h"

void trap() {
	sbi_ecall_console_puts("trap() called\n");
	while (true) { wfi(); }
}

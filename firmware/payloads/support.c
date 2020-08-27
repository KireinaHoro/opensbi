#include "common.h"
#include "nvdla.h"

irq_handler_t handlers[] = {NULL, NULL, &handle_dla};

void delay() {
    for (int i = 0; i < 100 * 1000 * 1000; ++i) {
        asm volatile("nop");
    }
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

int _write(int file, char *ptr, int len) {
    int i;
    file = file;
    for (i = 0; i < len; i++) {
        sbi_ecall_console_putc(*ptr++);
    }
    return len;
}

void _exit(int n) {
    sbi_ecall_console_puts("exit(");
    sbi_ecall_console_printhex(n);
    sbi_ecall_console_puts(") called, halting\n");
    while (true) {
        wfi();
    }
}

uint64_t cpuid() { return r_tp(); }

// trap handler for illegal insn and intr
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
        printf("Interrupt! sip=%#lx sie=%#lx\n", r_sip(), r_sie());

        if ((scause & 0xff) == 0x9) {
            // claim IRQ from PLIC
            int hart = cpuid();
            int irq = *(uint32_t *)PLIC_SCLAIM(hart);
            printf("External interrupt IRQ %d\n", irq);

            if (irq > ELEMS(handlers)) {
                printf("Spurious IRQ\n");
            } else {
                (handlers[irq])();
            }

            *(uint32_t *)PLIC_SCLAIM(hart) = irq;
        }

        printf("Resuming...\n");
    }
}

void enable_irq(int *irqs, size_t count) {
    TRACE("enter")

    uint32_t senable = 0;
    for (size_t i = 0; i < count; ++i, ++irqs) {
        printf("Enabling IRQ #%d...\n", *irqs);
        *(uint32_t *)(PLIC + NVDLA_IRQ * 4) = 1;
        senable |= (1 << *irqs);
    }

    uint64_t hart = cpuid();
    *(uint32_t *)PLIC_SENABLE(hart) = senable;

    // set S-mode priority threshold to 0
    *(uint32_t *)PLIC_SPRIORITY(hart) = 0;

    TRACE("exit")
}

void hexdump(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

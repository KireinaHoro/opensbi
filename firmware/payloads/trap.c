#include <stdbool.h>

#include "common.h"
#include "routines.h"

// 0x40 byte cache blocks
#define CACHE_BLOCK 0x40UL

/* stvec needs to be aligned to 4-byte boundary */
__attribute__((optimize("align-functions=4"))) void trap() {
  // FIXME: proper flushing of L2
  // FIXME: get rid of hard-coded addresses
  volatile uint64_t *flush64 = (volatile uint64_t *)0x2010200;
  char buf[MAXLINE];

  sbi_ecall_console_printhex(read_csr(sepc));
  sbi_ecall_console_puts(EXIT_MAGIC);
  while (true) {
    uint64_t addr, len;
	sbi_getline(buf, MAXLINE - 1);
    sscanf(buf, "%ld%ld", &addr, &len);
    uint64_t right = addr + len;
    uint64_t first_block = addr & ~(CACHE_BLOCK - 1);
    uint64_t last_block = (right & ~(CACHE_BLOCK - 1)) == right
                              ? right
                              : (right & ~(CACHE_BLOCK - 1)) + CACHE_BLOCK;
    for (uint64_t p = first_block; p < last_block; p += CACHE_BLOCK) {
      *flush64 = p;
    }
    sbi_ecall_console_puts(FLUSH_MAGIC);
  }
}

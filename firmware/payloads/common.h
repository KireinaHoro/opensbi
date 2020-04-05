#include <stddef.h>
#include <stdint.h>

#define DDR_BASE 0x800000000L
#define DDR_SIZE 0x80000000L
#define SBI_SIZE 0x20000L

#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

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
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uint64_t)(va)) >> PXSHIFT(level)) & PXMASK)
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

void vminit();

// supervisor address translation and protection;
// holds the address of the page table.
static inline void w_satp(uint64 x) {
  asm volatile("csrw satp, %0" : : "r"(x));
}

// flush the TLB.
static inline void sfence_vma() {
  // the zero, zero means flush all TLB entries.
  asm volatile("sfence.vma zero, zero");
}

#define wfi()                                                                  \
  do {                                                                         \
    __asm__ __volatile__("wfi" ::: "memory");                                  \
  } while (0)

#define CALL_TRACE                                                             \
  { printf("enter %s\n", __func__); }

#define read_csr(reg)                                                          \
  ({                                                                           \
    unsigned long __tmp;                                                       \
    __asm__ volatile("csrr %0, " #reg : "=r"(__tmp));                          \
    __tmp;                                                                     \
  })

// \r\n will actually be written; ARM side checks for \r\n instead of \n
#define INIT_MAGIC "monitor-initialize\n"
#define EXIT_MAGIC "monitor-exit\n"

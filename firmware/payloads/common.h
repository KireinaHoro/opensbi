#define DDR_BASE 0x800000000L
#define DDR_SIZE 0x80000000L

#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

#define PTE_V (1L << 0) // valid
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)

void matmul_test();

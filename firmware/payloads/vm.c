// Virtual memory setup.  Most of the code are borrowed from xv6-riscv.
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

// Return the address of the PTE in page table pagetable
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
//
// The risc-v Sv39 scheme has three levels of page-table
// pages. A page-table page contains 512 64-bit PTEs.
// A 64-bit virtual address is split into five fields:
//   39..63 -- must be zero.
//   30..38 -- 9 bits of level-2 index.
//   21..39 -- 9 bits of level-1 index.
//   12..20 -- 9 bits of level-0 index.
//    0..12 -- 12 bits of byte offset within the page.
static pte_t *walk(pagetable_t pagetable, uint64 va, int alloc) {
    for (int level = 2; level > 0; level--) {
        pte_t *pte = &pagetable[PX(level, va)];
        if (*pte & PTE_V) {
            pagetable = (pagetable_t)PTE2PA(*pte);
        } else {
            if (!alloc || (pagetable = (pde_t *)memalign(PGSIZE, PGSIZE)) == 0)
                return 0;
            memset(pagetable, 0, PGSIZE);
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    return &pagetable[PX(0, va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned. Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
int _mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa,
              int perm) {
    uint64 a, last;
    pte_t *pte;

    a = PGROUNDDOWN(va);
    last = PGROUNDDOWN(va + size - 1);
    for (;;) {
        if ((pte = walk(pagetable, a, 1)) == 0)
            return -1;
        *pte = PA2PTE(pa) | perm | PTE_V;
        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

void mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa,
              int perm) {
    printf("pagetable %p: %p -> %p (size %p, perm %d)\n", (void *)pagetable,
           (void *)va, (void *)pa, (void *)size, perm);
    if (_mappages(pagetable, va, size, pa, perm)) {
        printf("failed to map %p -> %p (size %p, perm %d) in pagetable %p\n",
               (void *)va, (void *)pa, (void *)size, perm, pagetable);
        exit(-1);
    }
}

static pagetable_t pagetable;

void vminit() {
    pagetable = memalign(PGSIZE, PGSIZE);
    if (!pagetable) {
        perror("Failed to allocate page for pagetable");
        exit(-1);
    }
    memset(pagetable, 0, PGSIZE);
    printf("Creating pagetable at %p...\n", (void *)pagetable);

    // CLINT
    mappages(pagetable, CLINT, 0x10000, CLINT, PTE_R | PTE_W);

    // PLIC
    mappages(pagetable, PLIC, 0x400000, PLIC, PTE_R | PTE_W);

    // UART
    mappages(pagetable, UART, PGSIZE, UART, PTE_R | PTE_W);

    // DRAM
    mappages(pagetable, DDR_BASE + SBI_SIZE, DDR_SIZE - SBI_SIZE,
             DDR_BASE + SBI_SIZE, PTE_R | PTE_W | PTE_X | PTE_D | PTE_A);

    printf("Enabling VM...\n");
    w_satp(MAKE_SATP(pagetable));
    sfence_vma();

    printf("done.\n");
}

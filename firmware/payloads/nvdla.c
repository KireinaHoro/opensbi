#include "common.h"
#include "dla_regs.h"
#include "opendla_small.h"

void handle_dla() { TRACE("irq") }

void enable_dla() {
    uint32_t ver = glb_reg_read(S_NVDLA_HW_VERSION);
    printf("NVDLA Version %u\n", ver);
}

void reg_write(uint32_t addr, uint32_t reg) {
    uint32_t *a = (uint32_t *)NVDLA_BASE + addr;
    printf(">>> write %p\n", a);
    *a = reg;
}

uint32_t reg_read(uint32_t addr) {
    uint32_t *a = (uint32_t *)NVDLA_BASE + addr;
    printf(">>> read %p\n", a);
    return *a;
}

void dla_enable_intr(uint32_t mask) {
    uint32_t reg = glb_reg_read(S_INTR_MASK);

    reg = reg & (~mask);
    glb_reg_write(S_INTR_MASK, reg);
}

void dla_disable_intr(uint32_t mask) {
    uint32_t reg = glb_reg_read(S_INTR_MASK);

    reg = reg | mask;
    glb_reg_write(S_INTR_MASK, reg);
}

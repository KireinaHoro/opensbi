#include "common.h"
#include "dla_regs.h"
#include "opendla_small.h"

#include "nvdla.h"

void handle_dla() {
    TRACE("irq") 

    char *src_addr = (char *)SRC_ADDR;
    char *dst_addr = (char *)DST_ADDR;

    printf("Source string: %s\n", src_addr);
    printf("Destination string: %s\n", dst_addr);
}

void intr_test() {
    TRACE("enter")

    dla_enable_intr(MASK(GLB_S_INTR_MASK_0, SDP_DONE_MASK1) |
                    MASK(GLB_S_INTR_MASK_0, SDP_DONE_MASK0));

    glb_reg_write(S_INTR_SET, MASK(GLB_S_INTR_MASK_0, SDP_DONE_MASK0));

    TRACE("exit")
}

void enable_dla() {
    uint32_t ver = glb_reg_read(S_NVDLA_HW_VERSION);
    printf("NVDLA Version %u\n", ver);

    //dla_sdp_program();
    intr_test();
}

void reg_write(uint32_t addr, uint32_t reg) {
    uint32_t *a = (uint32_t *)NVDLA_BASE + addr;
    printf(">>> write %p <- %#x\n", a, reg);
    *a = reg;
}

uint32_t reg_read(uint32_t addr) {
    uint32_t *a = (uint32_t *)NVDLA_BASE + addr;
    printf(">>> read %p", a);
    fflush(stdout);
    uint32_t reg = *a;
    printf(" -> %#x\n", reg);
    return reg;
}

void dla_enable_intr(uint32_t mask) {
    TRACE("enter")

    printf("Requesting mask: %#x\n", mask);

    uint32_t reg = glb_reg_read(S_INTR_MASK);

    reg = reg & (~mask);
    glb_reg_write(S_INTR_MASK, reg);

    TRACE("exit")
}

void dla_disable_intr(uint32_t mask) {
    uint32_t reg = glb_reg_read(S_INTR_MASK);

    reg = reg | mask;
    glb_reg_write(S_INTR_MASK, reg);
}

#include "common.h"
#include "dla_regs.h"
#include "opendla_small.h"

#include "nvdla.h"

void handle_dla() {
    TRACE("irq")

    uint32_t mask;
    uint32_t reg;

    mask = glb_reg_read(S_INTR_MASK);
    reg = glb_reg_read(S_INTR_STATUS);

    printf("mask=%#x reg=%#x\n", mask, reg);

    if (reg & MASK(GLB_S_INTR_STATUS_0, CACC_DONE_STATUS0)) {
        TRACE("cacc status0")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, CACC_DONE_STATUS1)) {
        TRACE("cacc status1")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, SDP_DONE_STATUS0)) {
        TRACE("sdp status0")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, SDP_DONE_STATUS1)) {
        TRACE("sdp status1")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, CDP_DONE_STATUS0)) {
        TRACE("cdp status0")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, CDP_DONE_STATUS1)) {
        TRACE("cdp status1")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, RUBIK_DONE_STATUS0)) {
        TRACE("rubik status0")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, RUBIK_DONE_STATUS1)) {
        TRACE("rubik status1")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, PDP_DONE_STATUS0)) {
        TRACE("pdp status0")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, PDP_DONE_STATUS1)) {
        TRACE("pdp status1")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, BDMA_DONE_STATUS0)) {
        TRACE("bdma status0")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, BDMA_DONE_STATUS1)) {
        TRACE("bdma status1")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, CDMA_DAT_DONE_STATUS0)) {
        TRACE("cdma_dat status0")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, CDMA_DAT_DONE_STATUS1)) {
        TRACE("cdma_dat status1")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, CDMA_WT_DONE_STATUS0)) {
        TRACE("cdma_wt status0")
    }
    if (reg & MASK(GLB_S_INTR_STATUS_0, CDMA_WT_DONE_STATUS1)) {
        TRACE("cdma_wt status1")
    }

    char *src_addr = (char *)SRC_ADDR;
    char *dst_addr = (char *)DST_ADDR;

    printf("Source memory @ %p:\n", src_addr);
    hexdump(src_addr, DUMP_SIZE);

    printf("Destination memory @ %p:\n", dst_addr);
    hexdump(dst_addr, DUMP_SIZE);

    printf("Clearing interrupt...\n");

    glb_reg_write(S_INTR_STATUS, reg);

    mask = glb_reg_read(S_INTR_MASK);
    reg = glb_reg_read(S_INTR_STATUS);

    printf("mask=%#x reg=%#x\n", mask, reg);
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

    dla_sdp_program();
    // intr_test();
}

void reg_write(uint32_t addr, uint32_t reg) {
    uint32_t *a = (uint32_t *)(NVDLA_BASE + addr);
    printf(">>> write %p <- %#x\n", a, reg);
    *a = reg;
}

uint32_t reg_read(uint32_t addr) {
    uint32_t *a = (uint32_t *)(NVDLA_BASE + addr);
    printf(">>> read  %p", a);
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

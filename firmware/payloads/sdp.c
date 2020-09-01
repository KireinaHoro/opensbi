#include "common.h"
#include "dla_regs.h"
#include "nvdla.h"
#include "opendla_small.h"

#include <stdbool.h>
#include <string.h>

static const uint8_t map_ena[] = {
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DISABLE, YES),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DISABLE, NO),
};

static const uint8_t map_prelu[] = {
    FIELD_ENUM(SDP_D_DP_BS_CFG_0, BS_MUL_PRELU, NO),
    FIELD_ENUM(SDP_D_DP_BS_CFG_0, BS_MUL_PRELU, YES),
};

static const uint8_t map_bypass[] = {
    FIELD_ENUM(SDP_D_DP_BS_CFG_0, BS_BYPASS, YES),
    FIELD_ENUM(SDP_D_DP_BS_CFG_0, BS_BYPASS, NO),
};

static const uint8_t map_alu_op[] = {
    FIELD_ENUM(SDP_D_DP_EW_CFG_0, EW_ALU_ALGO, MAX),
    FIELD_ENUM(SDP_D_DP_EW_CFG_0, EW_ALU_ALGO, MIN),
    FIELD_ENUM(SDP_D_DP_EW_CFG_0, EW_ALU_ALGO, SUM),
    FIELD_ENUM(SDP_D_DP_EW_CFG_0, EW_ALU_ALGO, EQL),
};

static const uint8_t map_alu_src[] = {
    FIELD_ENUM(SDP_D_DP_BS_ALU_CFG_0, BS_ALU_SRC, MEM),
    FIELD_ENUM(SDP_D_DP_BS_ALU_CFG_0, BS_ALU_SRC, REG),
};

static const uint8_t map_fly[] = {
    FIELD_ENUM(SDP_D_FEATURE_MODE_CFG_0, FLYING_MODE, OFF),
    FIELD_ENUM(SDP_D_FEATURE_MODE_CFG_0, FLYING_MODE, ON),
};

static const uint8_t map_dst[] = {
    FIELD_ENUM(SDP_D_FEATURE_MODE_CFG_0, OUTPUT_DST, MEM),
    FIELD_ENUM(SDP_D_FEATURE_MODE_CFG_0, OUTPUT_DST, PDP),
};

static const uint8_t map_wg[] = {
    FIELD_ENUM(SDP_D_FEATURE_MODE_CFG_0, WINOGRAD, OFF),
    FIELD_ENUM(SDP_D_FEATURE_MODE_CFG_0, WINOGRAD, ON),
};

static const uint8_t map_precision[] = {
    FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, INT8),
    FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, INT16),
    FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, FP16),
};

static const uint32_t map_proc_precision[3][3] = {
    {
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, INT8),
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, INT8),
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, FP16),
    },
    {
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, INT8),
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, INT16),
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, FP16),
    },
    {
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, INT8),
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, INT16),
        FIELD_ENUM(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION, FP16),
    },
};

static const uint8_t map_op_type[] = {
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_USE, MUL),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_USE, MUL),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_USE, ALU),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_USE, BOTH),
};

static const uint8_t map_element_size[] = {
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_SIZE, ONE_BYTE),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_SIZE, TWO_BYTE),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_SIZE, TWO_BYTE),
};

static const uint8_t map_op_mode[] = {
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_MODE, PER_ELEMENT),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_MODE, PER_KERNEL),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DATA_MODE, PER_ELEMENT),
};

static const uint8_t map_ram_type[] = {
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_RAM_TYPE, MC),
    FIELD_ENUM(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_RAM_TYPE, CV),
};

static const uint8_t map_perf_dma[] = {
    FIELD_ENUM(SDP_D_PERF_ENABLE_0, PERF_DMA_EN, NO),
    FIELD_ENUM(SDP_D_PERF_ENABLE_0, PERF_DMA_EN, YES),
};

static const uint8_t map_perf_lut[] = {
    FIELD_ENUM(SDP_D_PERF_ENABLE_0, PERF_LUT_EN, NO),
    FIELD_ENUM(SDP_D_PERF_ENABLE_0, PERF_LUT_EN, YES),
};

static const uint8_t map_perf_sat[] = {
    FIELD_ENUM(SDP_D_PERF_ENABLE_0, PERF_SAT_EN, NO),
    FIELD_ENUM(SDP_D_PERF_ENABLE_0, PERF_SAT_EN, YES),
};

static const uint8_t map_perf_nan_inf[] = {
    FIELD_ENUM(SDP_D_PERF_ENABLE_0, PERF_NAN_INF_COUNT_EN, NO),
    FIELD_ENUM(SDP_D_PERF_ENABLE_0, PERF_NAN_INF_COUNT_EN, YES),
};

void dla_sdp_set_producer(int32_t group_id, int32_t rdma_group_id) {
    uint32_t reg;

    /**
     * set producer pointer for all sub-modules
     */
    reg = group_id << SHIFT(SDP_S_POINTER_0, PRODUCER);
    sdp_reg_write(S_POINTER, reg);
    reg = rdma_group_id << SHIFT(SDP_RDMA_S_POINTER_0, PRODUCER);
    sdp_rdma_reg_write(S_POINTER, reg);
}

void dla_sdp_enable(bool is_rdma_needed) {
    uint32_t reg;

    TRACE("enter")

    /**
     * enable all sub-modules
     */
    if (is_rdma_needed) {
        reg = FIELD_ENUM(SDP_RDMA_D_OP_ENABLE_0, OP_EN, ENABLE);
        sdp_rdma_reg_write(D_OP_ENABLE, reg);
    }
    reg = FIELD_ENUM(SDP_D_OP_ENABLE_0, OP_EN, ENABLE);
    sdp_reg_write(D_OP_ENABLE, reg);

    TRACE("exit")
}

void processor_sdp_program(uint64_t src_addr, uint64_t dst_addr, uint32_t batch,
                           uint32_t src_w, uint32_t src_h, uint32_t src_c,
                           uint32_t line_stride, uint32_t surf_stride,
                           uint32_t batch_stride) {
    TRACE("enter")

    uint32_t reg, high, low;
    bool fly = src_addr == 0;
    bool out_dma_ena = dst_addr != 0;

    if (src_addr == 0) {
        TRACE("input fly-by")
    } else {
        TRACE("input from RAM")
        CHECK_ALIGN(src_addr, atom_size);
    }

    if (out_dma_ena) {
        TRACE("output to RAM")
        CHECK_ALIGN(dst_addr, atom_size);
    } else {
        TRACE("output fly-by")
    }

    uint32_t conv_mode = 0;     // no winograd
    uint32_t src_precision = 0; // int8
    uint32_t dst_precision = 0; // int8
    uint32_t mcif = 0;          // MC instead of CV (no secondary SRAM IF)
    uint32_t bypass = 0;        // bypass

    // clear old rdma state (?)
    reg = (map_fly[0] << SHIFT(SDP_RDMA_D_FEATURE_MODE_CFG_0, FLYING_MODE));
    sdp_rdma_reg_write(D_FEATURE_MODE_CFG, reg);

    reg = (map_ena[1] << SHIFT(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DISABLE));
    sdp_rdma_reg_write(D_BRDMA_CFG, reg);
    reg = (map_ena[1] << SHIFT(SDP_RDMA_D_NRDMA_CFG_0, NRDMA_DISABLE));
    sdp_rdma_reg_write(D_NRDMA_CFG, reg);
    reg = (map_ena[1] << SHIFT(SDP_RDMA_D_ERDMA_CFG_0, ERDMA_DISABLE));
    sdp_rdma_reg_write(D_ERDMA_CFG, reg);

    // sdp rdma configuration
    reg =
        (map_fly[fly] << SHIFT(SDP_RDMA_D_FEATURE_MODE_CFG_0, FLYING_MODE)) |
        (map_wg[conv_mode] << SHIFT(SDP_RDMA_D_FEATURE_MODE_CFG_0, WINOGRAD)) |
        (map_precision[src_precision]
         << SHIFT(SDP_RDMA_D_FEATURE_MODE_CFG_0, IN_PRECISION)) |
        (map_precision[dst_precision]
         << SHIFT(SDP_RDMA_D_FEATURE_MODE_CFG_0, OUT_PRECISION)) |
        (map_proc_precision[dst_precision][src_precision]
         << SHIFT(SDP_RDMA_D_FEATURE_MODE_CFG_0, PROC_PRECISION)) |
        ((batch - 1) << SHIFT(SDP_RDMA_D_FEATURE_MODE_CFG_0, BATCH_NUMBER));
    sdp_rdma_reg_write(D_FEATURE_MODE_CFG, reg);

    // rdma is needed?
    if (!fly) {
        TRACE("enabling RDMA")

        sdp_rdma_reg_write(D_DATA_CUBE_WIDTH, src_w - 1);
        sdp_rdma_reg_write(D_DATA_CUBE_HEIGHT, src_h - 1);
        sdp_rdma_reg_write(D_DATA_CUBE_CHANNEL, src_c - 1);

        /* config SDP source info */
        high = HIGH32BITS(src_addr);
        low = LOW32BITS(src_addr);
        sdp_rdma_reg_write(D_SRC_BASE_ADDR_LOW, low);
        sdp_rdma_reg_write(D_SRC_BASE_ADDR_HIGH, high);
        sdp_rdma_reg_write(D_SRC_LINE_STRIDE, line_stride);
        sdp_rdma_reg_write(D_SRC_SURFACE_STRIDE, surf_stride);
        sdp_rdma_reg_write(D_SRC_DMA_CFG, map_ram_type[mcif]);

        // x1 dma is disabled
        reg = (map_ena[0] << SHIFT(SDP_RDMA_D_BRDMA_CFG_0, BRDMA_DISABLE));
        sdp_rdma_reg_write(D_BRDMA_CFG, reg);

        // x2 dma is disabled
        reg = (map_ena[0] << SHIFT(SDP_RDMA_D_NRDMA_CFG_0, NRDMA_DISABLE));
        sdp_rdma_reg_write(D_NRDMA_CFG, reg);

        // y dma is disabled
        reg = (map_ena[0] << SHIFT(SDP_RDMA_D_ERDMA_CFG_0, ERDMA_DISABLE));
        sdp_rdma_reg_write(D_ERDMA_CFG, reg);
    }

    sdp_reg_write(D_DATA_CUBE_WIDTH, src_w - 1);
    sdp_reg_write(D_DATA_CUBE_HEIGHT, src_h - 1);
    sdp_reg_write(D_DATA_CUBE_CHANNEL, src_c - 1);

    // wdma is needed?
    if (out_dma_ena) {
        high = HIGH32BITS(dst_addr);
        low = LOW32BITS(dst_addr);
        sdp_reg_write(D_DST_BASE_ADDR_HIGH, high);
        sdp_reg_write(D_DST_BASE_ADDR_LOW, low);
        sdp_reg_write(D_DST_LINE_STRIDE, line_stride);
        sdp_reg_write(D_DST_SURFACE_STRIDE, surf_stride);
    }

    // BS (x1) - bypassed
    reg = (map_bypass[bypass] << SHIFT(SDP_D_DP_BS_CFG_0, BS_BYPASS));
    sdp_reg_write(D_DP_BS_CFG, reg);

    // BN (x2) - bypassed
    reg = (map_bypass[bypass] << SHIFT(SDP_D_DP_BN_CFG_0, BN_BYPASS));
    sdp_reg_write(D_DP_BN_CFG, reg);

    // EW (y) - bypassed
    reg = (map_bypass[bypass] << SHIFT(SDP_D_DP_EW_CFG_0, EW_BYPASS));
    sdp_reg_write(D_DP_EW_CFG, reg);

    // sdp configuration
    reg =
        (map_fly[fly] << SHIFT(SDP_D_FEATURE_MODE_CFG_0, FLYING_MODE)) |
        (map_dst[!out_dma_ena] << SHIFT(SDP_D_FEATURE_MODE_CFG_0, OUTPUT_DST)) |
        (map_wg[conv_mode] << SHIFT(SDP_D_FEATURE_MODE_CFG_0, WINOGRAD)) |
        ((batch - 1) << SHIFT(SDP_D_FEATURE_MODE_CFG_0, BATCH_NUMBER));
    sdp_reg_write(D_FEATURE_MODE_CFG, reg);
    sdp_reg_write(D_DST_DMA_CFG, map_ram_type[mcif]);
    if (batch > 1)
        sdp_reg_write(D_DST_BATCH_STRIDE, batch_stride);

    reg = (map_proc_precision[dst_precision][src_precision]
           << SHIFT(SDP_D_DATA_FORMAT_0, PROC_PRECISION)) |
          (map_precision[dst_precision]
           << SHIFT(SDP_D_DATA_FORMAT_0, OUT_PRECISION));
    sdp_reg_write(D_DATA_FORMAT, reg);

    // bypass out converter
    sdp_reg_write(D_CVT_OFFSET, 0);
    sdp_reg_write(D_CVT_SCALE, 1);
    sdp_reg_write(D_CVT_SHIFT, 0);

    TRACE("exit")
}

const char *test_string =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do";

void dla_sdp_program() {
    TRACE("SDP data movement test")

    uint64_t src_addr = SRC_ADDR;
    uint64_t dst_addr = DST_ADDR;
    size_t len = strlen(test_string) + 1;

    printf("Moving test string from %#lx to %#lx...\n", src_addr, dst_addr);

    uint32_t batch = 1;
    uint32_t w = 2;
    uint32_t h = 1;
    // channel are in units of atoms
    uint32_t c = 2 * atom_size;

    // packed: need to multiply by atom size
    uint32_t ls = w * atom_size;
    uint32_t ss = w * h * atom_size;
    uint32_t bs = batch;

    // set marker bytes
    memset((void *)dst_addr, 0xcc, DUMP_SIZE);
    memset((void *)src_addr, 0xdd, DUMP_SIZE);

    // copy src string
    memcpy((void *)src_addr, test_string, len);

    // set producer to 0 - we don't do interleave
    dla_sdp_set_producer(0, 0);

    // enable interrupt
    dla_enable_intr(MASK(GLB_S_INTR_MASK_0, SDP_DONE_MASK1) |
                    MASK(GLB_S_INTR_MASK_0, SDP_DONE_MASK0));

    // program sdp
    processor_sdp_program(src_addr, dst_addr, batch, w, h, c, ls, ss, bs);

    // set producer to 0 - we don't do interleave
    dla_sdp_set_producer(0, 0);

    // enable all units
    dla_sdp_enable(src_addr != 0);

    TRACE("exit")
}

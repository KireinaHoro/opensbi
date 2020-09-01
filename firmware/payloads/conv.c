#include "common.h"
#include "dla_regs.h"
#include "nvdla.h"
#include "opendla_small.h"

#include <stdbool.h>
#include <string.h>

static const uint8_t map_precision[] = {
    FIELD_ENUM(CDMA_D_MISC_CFG_0, IN_PRECISION, INT8),
    FIELD_ENUM(CDMA_D_MISC_CFG_0, IN_PRECISION, INT16),
    FIELD_ENUM(CDMA_D_MISC_CFG_0, IN_PRECISION, FP16),
};

static const uint8_t map_conv[] = {
    FIELD_ENUM(CACC_D_MISC_CFG_0, CONV_MODE, DIRECT),
    FIELD_ENUM(CACC_D_MISC_CFG_0, CONV_MODE, WINOGRAD),
};

static const uint8_t map_weight_fmt[] = {
    FIELD_ENUM(CSC_D_WEIGHT_FORMAT_0, WEIGHT_FORMAT, UNCOMPRESSED),
    FIELD_ENUM(CSC_D_WEIGHT_FORMAT_0, WEIGHT_FORMAT, COMPRESSED),
};

static const uint8_t map_img_fmt[][2] = {
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R8), 1},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R10), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R12), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R16), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R16_I), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R16_F), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A16B16G16R16), 8},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_X16B16G16R16), 8},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A16B16G16R16_F), 8},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A16Y16U16V16), 8},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_V16U16Y16A16), 8},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A16Y16U16V16_F), 8},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A8B8G8R8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A8R8G8B8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_B8G8R8A8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R8G8B8A8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_X8B8G8R8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_X8R8G8B8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_B8G8R8X8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R8G8B8X8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A2B10G10R10), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A2R10G10B10), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_B10G10R10A2), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_R10G10B10A2), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A2Y10U10V10), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_V10U10Y10A2), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_A8Y8U8V8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_V8U8Y8A8), 4},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_Y8___U8V8_N444), 1},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_Y8___V8U8_N444), 1},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_Y10___U10V10_N444), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_Y10___V10U10_N444), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_Y12___U12V12_N444), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_Y12___V12U12_N444), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_Y16___U16V16_N444), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT, T_Y16___V16U16_N444), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, DATAIN_FORMAT, FEATURE), 2},
    {FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, DATAIN_FORMAT, PIXEL), 1},
};

static const uint8_t map_pixel[] = {
    FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, PIXEL_MAPPING, PITCH_LINEAR),
};

static const uint8_t map_ram[] = {
    FIELD_ENUM(CDMA_D_DAIN_RAM_TYPE_0, DATAIN_RAM_TYPE, MCIF),
    FIELD_ENUM(CDMA_D_DAIN_RAM_TYPE_0, DATAIN_RAM_TYPE, CVIF),
};

static const uint8_t map_mean[] = {
    FIELD_ENUM(CDMA_D_MEAN_FORMAT_0, MEAN_FORMAT, DISABLE),
    FIELD_ENUM(CDMA_D_MEAN_FORMAT_0, MEAN_FORMAT, ENABLE),
};

static uint32_t get_in_format(uint8_t format) {
    uint32_t in_format = 0;

    if (format >= FORMAT_T_R8 && format < FORMAT_FEATURE) {
        in_format = FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, DATAIN_FORMAT, PIXEL);
    } else if (format == FORMAT_FEATURE) {
        in_format = FIELD_ENUM(CDMA_D_DATAIN_FORMAT_0, DATAIN_FORMAT, FEATURE);
    } else {
        assert(0);
    }

    return in_format;
}

void dla_conv_set_producer(int32_t group_id, int32_t rdma_group_id) {
    uint32_t reg;

    /* set producer pointer for all sub-modules */
    reg = group_id << SHIFT(CACC_S_POINTER_0, PRODUCER);
    cacc_reg_write(S_POINTER, reg);
    cmac_a_reg_write(S_POINTER, reg);
    cmac_b_reg_write(S_POINTER, reg);
    csc_reg_write(S_POINTER, reg);
    cdma_reg_write(S_POINTER, reg);
}

void dla_conv_enable() {
    uint32_t reg;

    TRACE("enter")

    do {
        reg = cdma_reg_read(S_CBUF_FLUSH_STATUS);
    } while (!(reg & MASK(CDMA_S_CBUF_FLUSH_STATUS_0, FLUSH_DONE)));

    /* enable all sub-modules */
    reg = FIELD_ENUM(CACC_D_OP_ENABLE_0, OP_EN, ENABLE);
    cacc_reg_write(D_OP_ENABLE, reg);
    cmac_a_reg_write(D_OP_ENABLE, reg);
    cmac_b_reg_write(D_OP_ENABLE, reg);
    csc_reg_write(D_OP_ENABLE, reg);
    cdma_reg_write(D_OP_ENABLE, reg);

    TRACE("exit")
}

void processor_conv_program(
    uint64_t weight_address, uint64_t weight_size, uint64_t input_address,
    uint64_t output_address, uint32_t batch, uint32_t batch_stride,
    uint32_t dst_w, uint32_t dst_h, uint32_t dst_c, uint32_t dst_line_stride,
    uint32_t dst_surf_stride, uint32_t src_w, uint32_t src_h, uint32_t src_c,
    uint32_t src_line_stride, uint32_t src_surf_stride, uint32_t kern_w,
    uint32_t kern_h, uint32_t kern_c, uint32_t conv_stride_x,
    uint32_t conv_stride_y, uint32_t pad_x_left, uint32_t pad_x_right,
    uint32_t pad_y_top, uint32_t pad_y_bottom, uint8_t pad_val,
    uint32_t dilation_x, uint32_t dilation_y) {
    TRACE("enter")

    uint32_t reg, high, low, shift, mask;
    uint8_t gid = 0; // always use group 0

    uint32_t conv_mode = 0;     // direct convolution
    uint32_t in_precision = 0;  // int8
    uint32_t out_precision = 0; // int8
    uint8_t data_reuse = 0;
    uint8_t weight_reuse = 0;
    uint8_t data_format = FORMAT_FEATURE;
    uint8_t pixel_mapping = 0;
    uint8_t pixel_override = 0;
    uint8_t post_extension = 0;
    uint8_t weight_format = 0;
    uint8_t src_type = 0;    // MCIF
    uint8_t weight_type = 0; // MCIF
    uint8_t mean_format = 0; // mean registers not used

    // TODO: figure out following from compiler (ConvCoreNode.cpp)
    uint8_t skip_data_rls = 0;
    uint8_t skip_weight_rls = 0;
    uint32_t entry_per_slice = 0;
    uint32_t release = 0;
    uint32_t data_bank = 0;
    uint32_t weight_bank = 0;
    uint32_t offset_u = 0;
    uint32_t in_line_uv_stride = 0;
    uint32_t fetch_grain = 0;
    uint32_t bytes_per_kernel = 0;
    uint32_t weight_bytes = 0;

    if (weight_address != 0) {
        CHECK_ALIGN(weight_address, atom_size);
        CHECK_ALIGN(weight_size, 128);
    }

    if (output_address != 0) {
        CHECK_ALIGN(output_address, atom_size);
        CHECK_ALIGN(dst_line_stride, atom_size);
        CHECK_ALIGN(dst_surf_stride, atom_size);
    }

    CHECK_ALIGN(input_address, atom_size);

    assert(conv_op->out_cvt.scale == 1);
    assert(conv_op->out_cvt.offset == 0);

    /* check if the register group is idle */
    reg = cacc_reg_read(S_STATUS);
    mask =
        gid ? MASK(CACC_S_STATUS_0, STATUS_1) : MASK(CACC_S_STATUS_0, STATUS_0);
    shift = gid ? SHIFT(CACC_S_STATUS_0, STATUS_1)
                : SHIFT(CACC_S_STATUS_0, STATUS_0);
    reg = (reg & mask) >> shift;
    assert(reg == FIELD_ENUM(CACC_S_STATUS_0, STATUS_0, IDLE));

    reg = cmac_a_reg_read(S_STATUS);
    mask = gid ? MASK(CMAC_A_S_STATUS_0, STATUS_1)
               : MASK(CMAC_A_S_STATUS_0, STATUS_0);
    shift = gid ? SHIFT(CMAC_A_S_STATUS_0, STATUS_1)
                : SHIFT(CMAC_A_S_STATUS_0, STATUS_0);
    reg = (reg & mask) >> shift;
    assert(reg == FIELD_ENUM(CMAC_A_S_STATUS_0, STATUS_0, IDLE));

    reg = cmac_b_reg_read(S_STATUS);
    mask = gid ? MASK(CMAC_B_S_STATUS_0, STATUS_1)
               : MASK(CMAC_B_S_STATUS_0, STATUS_0);
    shift = gid ? SHIFT(CMAC_B_S_STATUS_0, STATUS_1)
                : SHIFT(CMAC_B_S_STATUS_0, STATUS_0);
    reg = (reg & mask) >> shift;
    assert(reg == FIELD_ENUM(CMAC_B_S_STATUS_0, STATUS_0, IDLE));

    reg = csc_reg_read(S_STATUS);
    mask =
        gid ? MASK(CSC_S_STATUS_0, STATUS_1) : MASK(CSC_S_STATUS_0, STATUS_0);
    shift =
        gid ? SHIFT(CSC_S_STATUS_0, STATUS_1) : SHIFT(CSC_S_STATUS_0, STATUS_0);
    reg = (reg & mask) >> shift;
    assert(reg == FIELD_ENUM(CSC_S_STATUS_0, STATUS_0, IDLE));

    reg = cdma_reg_read(S_STATUS);
    mask =
        gid ? MASK(CDMA_S_STATUS_0, STATUS_1) : MASK(CDMA_S_STATUS_0, STATUS_0);
    shift = gid ? SHIFT(CDMA_S_STATUS_0, STATUS_1)
                : SHIFT(CDMA_S_STATUS_0, STATUS_0);
    reg = (reg & mask) >> shift;
    assert(reg == FIELD_ENUM(CDMA_S_STATUS_0, STATUS_0, IDLE));

    /* reverse config each sub-module in CC */

    /* CACC */
    reg = (map_conv[conv_mode] << SHIFT(CACC_D_MISC_CFG_0, CONV_MODE)) |
          (map_precision[out_precision]
           << SHIFT(CACC_D_MISC_CFG_0, PROC_PRECISION));
    cacc_reg_write(D_MISC_CFG, reg);

    reg = ((dst_w - 1) << SHIFT(CACC_D_DATAOUT_SIZE_0_0, DATAOUT_WIDTH)) |
          ((dst_h - 1) << SHIFT(CACC_D_DATAOUT_SIZE_0_0, DATAOUT_HEIGHT));
    cacc_reg_write(D_DATAOUT_SIZE_0, reg);

    reg = ((dst_c - 1) << SHIFT(CACC_D_DATAOUT_SIZE_1_0, DATAOUT_CHANNEL));
    cacc_reg_write(D_DATAOUT_SIZE_1, reg);

    // some sort of wdma?
    low = LOW32BITS(output_address);
    cacc_reg_write(D_DATAOUT_ADDR, low);
    cacc_reg_write(D_BATCH_NUMBER, batch - 1);
    cacc_reg_write(D_LINE_STRIDE, dst_line_stride);
    cacc_reg_write(D_SURF_STRIDE, dst_surf_stride);

    if (dst_w == 1 && dst_h == 1) {
        assert(dst_line_stride == dst_w * atom_size);
        reg = (CACC_D_DATAOUT_MAP_0_LINE_PACKED_TRUE
               << SHIFT(CACC_D_DATAOUT_MAP_0, LINE_PACKED));
        reg |= (CACC_D_DATAOUT_MAP_0_SURF_PACKED_TRUE
                << SHIFT(CACC_D_DATAOUT_MAP_0, SURF_PACKED));
    } else {
        reg = (FIELD_ENUM(CACC_D_DATAOUT_MAP_0, LINE_PACKED, FALSE)
               << SHIFT(CACC_D_DATAOUT_MAP_0, LINE_PACKED));
        reg |= (FIELD_ENUM(CACC_D_DATAOUT_MAP_0, SURF_PACKED, FALSE)
                << SHIFT(CACC_D_DATAOUT_MAP_0, SURF_PACKED));
    }
    cacc_reg_write(D_DATAOUT_MAP, reg);

    cacc_reg_write(D_CLIP_CFG, conv_op->out_cvt.truncate);

    /* CMAC */
    reg = (map_conv[conv_mode] << SHIFT(CMAC_A_D_MISC_CFG_0, CONV_MODE)) |
          (map_precision[out_precision]
           << SHIFT(CMAC_A_D_MISC_CFG_0, PROC_PRECISION));
    cmac_a_reg_write(D_MISC_CFG, reg);
    cmac_b_reg_write(D_MISC_CFG, reg);

    /* CSC */
    reg = (map_conv[conv_mode] << SHIFT(CSC_D_MISC_CFG_0, CONV_MODE)) |
          (map_precision[out_precision]
           << SHIFT(CSC_D_MISC_CFG_0, IN_PRECISION)) |
          (map_precision[out_precision]
           << SHIFT(CSC_D_MISC_CFG_0, PROC_PRECISION)) |
          (data_reuse << SHIFT(CSC_D_MISC_CFG_0, DATA_REUSE)) |
          (weight_reuse << SHIFT(CSC_D_MISC_CFG_0, WEIGHT_REUSE)) |
          (skip_data_rls << SHIFT(CSC_D_MISC_CFG_0, SKIP_DATA_RLS)) |
          (skip_weight_rls << SHIFT(CSC_D_MISC_CFG_0, SKIP_WEIGHT_RLS));
    csc_reg_write(D_MISC_CFG, reg);

    reg = (get_in_format(data_format)
           << SHIFT(CSC_D_DATAIN_FORMAT_0, DATAIN_FORMAT));
    csc_reg_write(D_DATAIN_FORMAT, reg);

    reg = ((src_w - 1) << SHIFT(CSC_D_DATAIN_SIZE_EXT_0_0, DATAIN_WIDTH_EXT)) |
          ((src_h - 1) << SHIFT(CSC_D_DATAIN_SIZE_EXT_0_0, DATAIN_HEIGHT_EXT));
    csc_reg_write(D_DATAIN_SIZE_EXT_0, reg);

    reg = ((src_c - 1) << SHIFT(CSC_D_DATAIN_SIZE_EXT_1_0, DATAIN_CHANNEL_EXT));
    csc_reg_write(D_DATAIN_SIZE_EXT_1, reg);

    reg = ((batch - 1) << SHIFT(CSC_D_BATCH_NUMBER_0, BATCHES));
    csc_reg_write(D_BATCH_NUMBER, reg);
    reg = ((post_extension) << SHIFT(CSC_D_POST_Y_EXTENSION_0, Y_EXTENSION));
    csc_reg_write(D_POST_Y_EXTENSION, reg);

    reg = ((entry_per_slice - 1) << SHIFT(CSC_D_ENTRY_PER_SLICE_0, ENTRIES));
    csc_reg_write(D_ENTRY_PER_SLICE, reg);

    reg = (map_weight_fmt[weight_format]
           << SHIFT(CSC_D_WEIGHT_FORMAT_0, WEIGHT_FORMAT));
    csc_reg_write(D_WEIGHT_FORMAT, reg);

    reg = ((kern_w - 1) << SHIFT(CSC_D_WEIGHT_SIZE_EXT_0_0, WEIGHT_WIDTH_EXT)) |
          ((kern_h - 1) << SHIFT(CSC_D_WEIGHT_SIZE_EXT_0_0, WEIGHT_HEIGHT_EXT));
    csc_reg_write(D_WEIGHT_SIZE_EXT_0, reg);

    reg =
        ((kern_c - 1) << SHIFT(CSC_D_WEIGHT_SIZE_EXT_1_0, WEIGHT_CHANNEL_EXT)) |
        ((dst_c - 1) << SHIFT(CSC_D_WEIGHT_SIZE_EXT_1_0, WEIGHT_KERNEL));
    csc_reg_write(D_WEIGHT_SIZE_EXT_1, reg);

    csc_reg_write(D_WEIGHT_BYTES, weight_size);
    // we don't do weight compression
    csc_reg_write(D_WMB_BYTES, 0);

    reg = ((dst_w - 1) << SHIFT(CSC_D_DATAOUT_SIZE_0_0, DATAOUT_WIDTH)) |
          ((dst_h - 1) << SHIFT(CSC_D_DATAOUT_SIZE_0_0, DATAOUT_HEIGHT));
    csc_reg_write(D_DATAOUT_SIZE_0, reg);

    reg = ((dst_c - 1) << SHIFT(CSC_D_DATAOUT_SIZE_1_0, DATAOUT_CHANNEL));
    csc_reg_write(D_DATAOUT_SIZE_1, reg);

    reg = ((dst_w * dst_h - 1) << SHIFT(CSC_D_ATOMICS_0, ATOMICS));
    csc_reg_write(D_ATOMICS, reg);
    reg = ((release - 1) << SHIFT(CSC_D_RELEASE_0, RLS_SLICES));
    csc_reg_write(D_RELEASE, reg);

    reg = (conv_stride_x << SHIFT(CSC_D_CONV_STRIDE_EXT_0, CONV_X_STRIDE_EXT)) |
          (conv_stride_y << SHIFT(CSC_D_CONV_STRIDE_EXT_0, CONV_Y_STRIDE_EXT));
    csc_reg_write(D_CONV_STRIDE_EXT, reg);

    reg = ((dilation_x - 1) << SHIFT(CSC_D_DILATION_EXT_0, X_DILATION_EXT)) |
          ((dilation_y - 1) << SHIFT(CSC_D_DILATION_EXT_0, Y_DILATION_EXT));
    csc_reg_write(D_DILATION_EXT, reg);

    reg = (pad_x_left << SHIFT(CSC_D_ZERO_PADDING_0, PAD_LEFT)) |
          (pad_y_top << SHIFT(CSC_D_ZERO_PADDING_0, PAD_TOP));
    csc_reg_write(D_ZERO_PADDING, reg);

    reg = (pad_val << SHIFT(CSC_D_ZERO_PADDING_VALUE_0, PAD_VALUE)) &
          MASK(CSC_D_ZERO_PADDING_VALUE_0, PAD_VALUE);
    csc_reg_write(D_ZERO_PADDING_VALUE, reg);

    reg = ((data_bank - 1) << SHIFT(CSC_D_BANK_0, DATA_BANK)) |
          ((weight_bank - 1) << SHIFT(CSC_D_BANK_0, WEIGHT_BANK));
    csc_reg_write(D_BANK, reg);
    // we never do winograd
    csc_reg_write(D_PRA_CFG, 0);

    /* CBUF */
    /* there's no CBUF register */

    /* CDMA */
    reg = (map_conv[conv_mode] << SHIFT(CDMA_D_MISC_CFG_0, CONV_MODE)) |
          (map_precision[in_precision]
           << SHIFT(CDMA_D_MISC_CFG_0, IN_PRECISION)) |
          (map_precision[out_precision]
           << SHIFT(CDMA_D_MISC_CFG_0, PROC_PRECISION)) |
          (data_reuse << SHIFT(CDMA_D_MISC_CFG_0, DATA_REUSE)) |
          (weight_reuse << SHIFT(CDMA_D_MISC_CFG_0, WEIGHT_REUSE)) |
          (skip_data_rls << SHIFT(CDMA_D_MISC_CFG_0, SKIP_DATA_RLS)) |
          (skip_weight_rls << SHIFT(CDMA_D_MISC_CFG_0, SKIP_WEIGHT_RLS));
    cdma_reg_write(D_MISC_CFG, reg);

    reg =
        (get_in_format(data_format)
         << SHIFT(CDMA_D_DATAIN_FORMAT_0, DATAIN_FORMAT)) |
        (map_img_fmt[data_format][0]
         << SHIFT(CDMA_D_DATAIN_FORMAT_0, PIXEL_FORMAT)) |
        (map_pixel[pixel_mapping]
         << SHIFT(CDMA_D_DATAIN_FORMAT_0, PIXEL_MAPPING)) |
        (pixel_override << SHIFT(CDMA_D_DATAIN_FORMAT_0, PIXEL_SIGN_OVERRIDE));
    cdma_reg_write(D_DATAIN_FORMAT, reg);

    reg = ((src_w - 1) << SHIFT(CDMA_D_DATAIN_SIZE_0_0, DATAIN_WIDTH)) |
          ((src_h - 1) << SHIFT(CDMA_D_DATAIN_SIZE_0_0, DATAIN_HEIGHT));
    cdma_reg_write(D_DATAIN_SIZE_0, reg);

    reg = ((src_c - 1) << SHIFT(CDMA_D_DATAIN_SIZE_1_0, DATAIN_CHANNEL));
    cdma_reg_write(D_DATAIN_SIZE_1, reg);

    reg = ((src_w - 1) << SHIFT(CDMA_D_DATAIN_SIZE_EXT_0_0, DATAIN_WIDTH_EXT)) |
          ((src_h - 1) << SHIFT(CDMA_D_DATAIN_SIZE_EXT_0_0, DATAIN_HEIGHT_EXT));
    cdma_reg_write(D_DATAIN_SIZE_EXT_0, reg);

    reg = (map_ram[src_type] << SHIFT(CDMA_D_DAIN_RAM_TYPE_0, DATAIN_RAM_TYPE));
    cdma_reg_write(D_DAIN_RAM_TYPE, reg);

    high = HIGH32BITS(input_address);
    low = LOW32BITS(input_address);
    cdma_reg_write(D_DAIN_ADDR_HIGH_0, high);
    cdma_reg_write(D_DAIN_ADDR_LOW_0, low);

    high = HIGH32BITS((input_address + offset_u));
    low = LOW32BITS(input_address + offset_u);
    cdma_reg_write(D_DAIN_ADDR_HIGH_1, high);
    cdma_reg_write(D_DAIN_ADDR_LOW_1, low);

    cdma_reg_write(D_LINE_STRIDE, src_line_stride);
    cdma_reg_write(D_SURF_STRIDE, src_surf_stride);
    cdma_reg_write(D_LINE_UV_STRIDE, in_line_uv_stride);

    reg = ((src_line_stride == (src_w * atom_size))
           << SHIFT(CDMA_D_DAIN_MAP_0, LINE_PACKED));
    reg |= ((src_surf_stride == (src_w * src_h * atom_size))
            << SHIFT(CDMA_D_DAIN_MAP_0, SURF_PACKED));
    cdma_reg_write(D_DAIN_MAP, reg);

    reg = ((batch - 1) << SHIFT(CDMA_D_BATCH_NUMBER_0, BATCHES));
    cdma_reg_write(D_BATCH_NUMBER, reg);

    cdma_reg_write(D_BATCH_STRIDE, batch_stride);

    reg = ((entry_per_slice - 1) << SHIFT(CDMA_D_ENTRY_PER_SLICE_0, ENTRIES));
    cdma_reg_write(D_ENTRY_PER_SLICE, reg);

    reg = ((fetch_grain - 1) << SHIFT(CDMA_D_FETCH_GRAIN_0, GRAINS));
    cdma_reg_write(D_FETCH_GRAIN, reg);

    reg = (map_weight_fmt[weight_format]
           << SHIFT(CDMA_D_WEIGHT_FORMAT_0, WEIGHT_FORMAT));
    cdma_reg_write(D_WEIGHT_FORMAT, reg);

    reg = ((bytes_per_kernel - 1)
           << SHIFT(CDMA_D_WEIGHT_SIZE_0_0, BYTE_PER_KERNEL));
    cdma_reg_write(D_WEIGHT_SIZE_0, reg);

    reg = ((dst_c - 1) << SHIFT(CDMA_D_WEIGHT_SIZE_1_0, WEIGHT_KERNEL));
    cdma_reg_write(D_WEIGHT_SIZE_1, reg);

    reg = (map_ram[weight_type]
           << SHIFT(CDMA_D_WEIGHT_RAM_TYPE_0, WEIGHT_RAM_TYPE));
    cdma_reg_write(D_WEIGHT_RAM_TYPE, reg);

    high = HIGH32BITS(weight_address);
    low = LOW32BITS(weight_address);
    cdma_reg_write(D_WEIGHT_ADDR_HIGH, high);
    cdma_reg_write(D_WEIGHT_ADDR_LOW, low);
    cdma_reg_write(D_WEIGHT_BYTES, weight_bytes);

    reg = (map_mean[mean_format] << SHIFT(CDMA_D_MEAN_FORMAT_0, MEAN_FORMAT));
    cdma_reg_write(D_MEAN_FORMAT, reg);

    // mean registers not used, skipped MEAN_GLOBAL_{0,1}

    // in converter not used
    reg = ((FIELD_ENUM(CDMA_D_CVT_CFG_0, CVT_EN, DISABLE))
           << SHIFT(CDMA_D_CVT_CFG_0, CVT_EN));
    cdma_reg_write(D_CVT_CFG, reg);

    reg = ((conv_stride_x - 1) << SHIFT(CDMA_D_CONV_STRIDE_0, CONV_X_STRIDE)) |
          ((conv_stride_y - 1) << SHIFT(CDMA_D_CONV_STRIDE_0, CONV_Y_STRIDE));
    cdma_reg_write(D_CONV_STRIDE, reg);

    reg = (pad_x_left << SHIFT(CDMA_D_ZERO_PADDING_0, PAD_LEFT)) |
          (pad_x_right << SHIFT(CDMA_D_ZERO_PADDING_0, PAD_RIGHT)) |
          (pad_y_top << SHIFT(CDMA_D_ZERO_PADDING_0, PAD_TOP)) |
          (pad_y_bottom << SHIFT(CDMA_D_ZERO_PADDING_0, PAD_BOTTOM));
    cdma_reg_write(D_ZERO_PADDING, reg);

    reg = pad_val << SHIFT(CDMA_D_ZERO_PADDING_VALUE_0, PAD_VALUE) &
          MASK(CDMA_D_ZERO_PADDING_VALUE_0, PAD_VALUE);
    cdma_reg_write(D_ZERO_PADDING_VALUE, reg);
    reg = ((weight_bank - 1) << SHIFT(CDMA_D_BANK_0, WEIGHT_BANK)) |
          ((data_bank - 1) << SHIFT(CDMA_D_BANK_0, DATA_BANK));
    cdma_reg_write(D_BANK, reg);

exit:
    dla_trace("Exit: %s", __func__);
    RETURN(ret);
}

int dla_conv_is_ready(struct dla_processor *processor,
                      struct dla_processor_group *group) {
    return 1;
}

void dla_conv_dump_config(struct dla_processor_group *group) {
    struct dla_conv_op_desc *conv_op;
    struct dla_conv_surface_desc *conv_surface;

    conv_surface = &group->surface_desc->conv_surface;
    conv_op = &group->operation_desc->conv_op;

    dla_debug_conv_surface_desc(conv_surface, group->roi_index);
    dla_debug_conv_op_desc(conv_op, group->roi_index);
}

int dla_conv_program(struct dla_processor_group *group) {
    int32_t ret;

    dla_trace("Enter: %s", __func__);

    ret = processor_conv_program(group);
    if (ret)
        goto exit;

exit:
    dla_trace("Exit: %s", __func__);
    RETURN(ret);
}

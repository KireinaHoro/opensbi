#pragma once

#include <assert.h>

// NVDLA base reg
#define NVDLA_BASE 0x10040000L

void enable_dla();
void handle_dla();

#define ATOM_SIZE 8

// SDP functions
void dla_sdp_program();
void dla_sdp_set_producer(int32_t group_id, int32_t rdma_group_id);
void dla_sdp_enable(bool is_rdma_needed);
void processor_sdp_program(uint64_t src_addr, uint64_t dst_addr, uint32_t batch,
                           uint32_t src_w, uint32_t src_h, uint32_t src_c,
                           uint32_t line_stride, uint32_t surf_stride,
                           uint32_t batch_stride);

// CONV functions
void dla_conv_program();
void dla_conv_set_producer(int32_t group_id, int32_t rdma_group_id);
void dla_conv_enable();
void processor_conv_program(
    uint64_t weight_address, uint64_t weight_size, uint64_t input_address,
    uint64_t output_address /* not used in hardware */, uint32_t batch,
    uint32_t batch_stride, uint32_t dst_w, uint32_t dst_h, uint32_t dst_c,
    uint32_t dst_line_stride, uint32_t dst_surf_stride, uint32_t src_w,
    uint32_t src_h, uint32_t src_c, uint32_t src_line_stride,
    uint32_t src_surf_stride, uint32_t kern_w, uint32_t kern_h, uint32_t kern_c,
    uint32_t conv_stride_x, uint32_t conv_stride_y, uint32_t pad_x_left,
    uint32_t pad_x_right, uint32_t pad_y_top, uint32_t pad_y_bottom,
    uint8_t pad_val, uint32_t dilation_x, uint32_t dilation_y);

void dla_enable_intr(uint32_t mask);

// test addresses for SDP - don't overlap with SBI code & data
#define SRC_ADDR 0x90010000L
#define DST_ADDR 0x90020000L

// test addresses for Conv
#define WEIGHT_ADDR 0x90010000L
#define INPUT_ADDR 0x90018000L
#define OUT_ADDR 0x90020000L

#define CHECK_ALIGN(val, align) assert((val & (align - 1)) == 0)
#define BITS(num, range) ((((0xFFFFFFFF >> (31 - (1 ? range))) & \
                            (0xFFFFFFFF << (0 ? range))) & num) >> \
                                        (0 ? range))
#define HIGH32BITS(val64bit) ((uint32_t)(val64bit >> 32))
#define LOW32BITS(val64bit) ((uint32_t)(val64bit))

// various constants used in original firmware

/**
 * @ingroup Convolution
 * @name Convolution mode
 * @brief Convolution modes support by DLA
 * @{
 */
#define CONV_MODE_DIRECT 0
#define CONV_MODE_WINOGRAD 1
/** @} */

/**
 * @ingroup Processors
 * @name Precision BPE mapping
 * @brief Precision formats and Bit Per Elements mapping
 * @{
 */
#define BPE_PRECISION_INT8 1
#define BPE_PRECISION_INT16 2
#define BPE_PRECISION_FP16 2
/** @} */

/**
 * @ingroup Processors
 * @name Precision types
 * @brief Precision formats supported by DLA engine
 * @{
 */
#define PRECISION_INT8 0
#define PRECISION_INT16 1
#define PRECISION_FP16 2
/** @} */

/**
 * @ingroup Processors
 * @name Data formats
 * @brief Data formats supported by DLA engine
 * @{
 */
#define FORMAT_T_R8 0
#define FORMAT_T_R10 1
#define FORMAT_T_R12 2
#define FORMAT_T_R16 3
#define FORMAT_T_R16_I 4
#define FORMAT_T_R16_F 5
#define FORMAT_T_A16B16G16R16 6
#define FORMAT_T_X16B16G16R16 7
#define FORMAT_T_A16B16G16R16_F 8
#define FORMAT_T_A16Y16U16V16 9
#define FORMAT_T_V16U16Y16A16 10
#define FORMAT_T_A16Y16U16V16_F 11
#define FORMAT_T_A8B8G8R8 12
#define FORMAT_T_A8R8G8B8 13
#define FORMAT_T_B8G8R8A8 14
#define FORMAT_T_R8G8B8A8 15
#define FORMAT_T_X8B8G8R8 16
#define FORMAT_T_X8R8G8B8 17
#define FORMAT_T_B8G8R8X8 18
#define FORMAT_T_R8G8B8X8 19
#define FORMAT_T_A2B10G10R10 20
#define FORMAT_T_A2R10G10B10 21
#define FORMAT_T_B10G10R10A2 22
#define FORMAT_T_R10G10B10A2 23
#define FORMAT_T_A2Y10U10V10 24
#define FORMAT_T_V10U10Y10A2 25
#define FORMAT_T_A8Y8U8V8 26
#define FORMAT_T_V8U8Y8A8 27
#define FORMAT_T_Y8___U8V8_N444 28
#define FORMAT_T_Y8___V8U8_N444 29
#define FORMAT_T_Y10___U10V10_N444 30
#define FORMAT_T_Y10___V10U10_N444 31
#define FORMAT_T_Y12___U12V12_N444 32
#define FORMAT_T_Y12___V12U12_N444 33
#define FORMAT_T_Y16___U16V16_N444 34
#define FORMAT_T_Y16___V16U16_N444 35
#define FORMAT_FEATURE 36
/** @} */

/**
 * @ingroup Convolution
 * @name Pixel mapping
 * @brief Pixel mapping formats supported for image input in Convolution
 * @{
 */
#define MAP_PITCH_LINEAR 0
/** @} */

/**
 * @ingroup Convolution
 * @name Weight formats
 * @brief Weight data formats supported in Convolution
 * @{
 */
#define WEIGHT_FORMAT_UNCOMPRESSED 0
#define WEIGHT_FORMAT_COMPRESSED 1
/** @} */

/**
 * @ingroup Convolution
 * @name Mean data format
 * @brief Mean data formats supported in Convolution
 * @{
 */
#define MEAN_FORMAT_DISABLE 0
#define MEAN_FORMAT_ENABLE 1
/** @} */

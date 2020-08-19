#pragma once

#include <assert.h>

// NVDLA base reg
#define NVDLA_BASE 0x10040000L

void enable_dla();
void handle_dla();

#define atom_size 8

void dla_sdp_program();

void dla_enable_intr(uint32_t mask);

// test addresses - don't overlap with SBI code & data
#define SRC_ADDR 0x90010000L
#define DST_ADDR 0x90020000L

#define CHECK_ALIGN(val, align)      assert((val&(align-1)) == 0)
#define BITS(num, range) ((((0xFFFFFFFF >> (31 - (1 ? range))) & \
                            (0xFFFFFFFF << (0 ? range))) & num) >> \
                                        (0 ? range))
#define HIGH32BITS(val64bit) ((uint32_t)(val64bit >> 32))
#define LOW32BITS(val64bit) ((uint32_t)(val64bit))

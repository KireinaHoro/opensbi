// See LICENSE for license details.

#ifndef SRC_MAIN_C_GEMMINI_H
#define SRC_MAIN_C_GEMMINI_H

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

#include "gemmini_params.h"

// #define GEMMINI_ASSERTIONS

// Matmul utility functions
void matmul(elem_t A[DIM][DIM], elem_t B[DIM][DIM], elem_t D[DIM][DIM],
	    int64_t C_full[DIM][DIM]);

void matmul_short(elem_t A[DIM][DIM], elem_t B[DIM][DIM], elem_t D[DIM][DIM],
		  elem_t C[DIM][DIM]);

void matmul_full(elem_t A[DIM][DIM], elem_t B[DIM][DIM], int64_t D[DIM][DIM],
		 int64_t C_full[DIM][DIM]);

void matadd(int64_t sum[DIM][DIM], int64_t m1[DIM][DIM], int64_t m2[DIM][DIM]);

// Rounding right shift equation: https://riscv.github.io/documents/riscv-v-spec/#_vector_fixed_point_rounding_mode_register_vxrm
#define ROUNDING_RIGHT_SHIFT(x, shift)                                        \
	({                                                                    \
		(shift) > 0                                                   \
			? (((x) >> (shift)) +                                 \
			   (((shift) == 0 ? 0 : (((x) >> ((shift)-1)) & 1)) & \
			    ((((shift) <= 1 ? 0                               \
					    : ((x) & ((1 << ((shift)-1)) -    \
						      1))) != 0) |            \
			     (((x) >> (shift)) & 1))))                        \
			: ((x) << (-(shift)));                                \
	})

// THIS IS A ROUNDING SHIFT! It also performs a saturating cast
void matshift(int64_t full[DIM][DIM], elem_t out[DIM][DIM], int shift);

void matrelu(elem_t in[DIM][DIM], elem_t out[DIM][DIM]);

void matrelu6(elem_t in[DIM][DIM], elem_t out[DIM][DIM], int scale);

void transpose(elem_t in[DIM][DIM], elem_t out[DIM][DIM]);

void printMatrix(elem_t m[DIM][DIM]);

int is_equal(elem_t x[DIM][DIM], elem_t y[DIM][DIM]);

// This is a GNU extension known as statment expressions
#define MAT_IS_EQUAL(dim_i, dim_j, x, y)                   \
	({                                                 \
		int result = 1;                            \
		for (size_t i = 0; i < dim_i; i++)         \
			for (size_t j = 0; j < dim_j; ++j) \
				if (x[i][j] != y[i][j]) {  \
					result = 0;        \
					break;             \
				}                          \
		result;                                    \
	})

int rand();

uint64_t read_cycles();

// Accelerator interface
#include "xcustom.h"

#define k_CONFIG 0
#define k_MVIN 2
#define k_MVOUT 3
#define k_COMPUTE_PRELOADED 4
#define k_COMPUTE_ACCUMULATE 5
#define k_PRELOAD 6
#define k_FLUSH 7
#define k_LOOP_WS 8

#define CONFIG_EX 0
#define CONFIG_LD 1
#define CONFIG_ST 2

#define XCUSTOM_ACC 3

#define GARBAGE_ADDR ((uint32_t)(-1))
#define OUTPUT_STATIONARY 0
#define WEIGHT_STATIONARY 1

#define NO_ACTIVATION 0
#define RELU 1
#define RELU6 2

#define ROCC_INSTRUCTION_RS1_RS2(x, rs1, rs2, funct) \
	ROCC_INSTRUCTION_0_R_R(x, rs1, rs2, funct, 10, 11)

// mvin and mvout
#define gemmini_extended_mvin(dram_addr, spad_addr, cols, rows)               \
		ROCC_INSTRUCTION_RS1_RS2(                                     \
			XCUSTOM_ACC, dram_addr,                               \
			((uint64_t)(rows) << (ADDR_LEN + 16)) |               \
				((uint64_t)(cols) << ADDR_LEN) | (spad_addr), \
			k_MVIN)

#define gemmini_block_mvin(dram_addr, spad_addr, len) \
	gemmini_extended_mvin(dram_addr, spad_addr, (len)*DIM, DIM)

#define gemmini_mvin(dram_addr, spad_addr) \
	gemmini_extended_mvin(dram_addr, spad_addr, DIM, DIM)

#define gemmini_extended_mvout(dram_addr, spad_addr, cols, rows)           \
		ROCC_INSTRUCTION_RS1_RS2(                                  \
			XCUSTOM_ACC, dram_addr,                            \
			((uint64_t)(rows) << (ADDR_LEN + 16)) |            \
				((uint64_t)(cols) << ADDR_LEN) |           \
				(uint64_t)(spad_addr),                     \
			k_MVOUT)

#define gemmini_mvout(dram_addr, spad_addr) \
	gemmini_extended_mvout(dram_addr, spad_addr, DIM, DIM)

// compute
#define gemmini_extended_compute_preloaded(A, BD, A_cols, A_rows, BD_cols,  \
					   BD_rows)                         \
	ROCC_INSTRUCTION_RS1_RS2(                                           \
		XCUSTOM_ACC,                                                \
		((uint64_t)(A_rows) << (ADDR_LEN + 16)) |                   \
			((uint64_t)(A_cols) << ADDR_LEN) | (uint64_t)(A),   \
		((uint64_t)(BD_rows) << (ADDR_LEN + 16)) |                  \
			((uint64_t)(BD_cols) << ADDR_LEN) | (uint64_t)(BD), \
		k_COMPUTE_PRELOADED)

#define gemmini_extended_compute_accumulated(A, BD, A_cols, A_rows, BD_cols, \
					     BD_rows)                        \
	ROCC_INSTRUCTION_RS1_RS2(                                            \
		XCUSTOM_ACC,                                                 \
		((uint64_t)(A_rows) << (ADDR_LEN + 16)) |                    \
			((uint64_t)(A_cols) << ADDR_LEN) | (uint64_t)(A),    \
		((uint64_t)(BD_rows) << (ADDR_LEN + 16)) |                   \
			((uint64_t)(BD_cols) << ADDR_LEN) | (uint64_t)(BD),  \
		k_COMPUTE_ACCUMULATE)

#define gemmini_compute_preloaded(A, BD) \
	gemmini_extended_compute_preloaded(A, BD, DIM, DIM, DIM, DIM)

#define gemmini_compute_accumulated(A, BD) \
	gemmini_extended_compute_accumulated(A, BD, DIM, DIM, DIM, DIM)

// preload
#define gemmini_extended_preload(BD, C, BD_cols, BD_rows, C_cols, C_rows)   \
	ROCC_INSTRUCTION_RS1_RS2(                                           \
		XCUSTOM_ACC,                                                \
		((uint64_t)(BD_rows) << (ADDR_LEN + 16)) |                  \
			((uint64_t)(BD_cols) << ADDR_LEN) | (uint64_t)(BD), \
		((uint64_t)(C_rows) << (ADDR_LEN + 16)) |                   \
			((uint64_t)(C_cols) << ADDR_LEN) | (uint64_t)(C),   \
		k_PRELOAD)

#define gemmini_preload(BD, C) \
	gemmini_extended_preload(BD, C, DIM, DIM, DIM, DIM)

#define gemmini_preload_zeros(C) gemmini_preload(GARBAGE_ADDR, C)

// weight-stationary matmul loop
/*
#define gemmini_loop_ws(A, B, I, J, K, bias) \
    ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(B) << 32) | (A), ((uint64_t)(bias) << 48) | ((uint64_t)(K) << 32) | ((J) << 16) | (I), k_LOOP_WS)
*/

// config
#define gemmini_config_ex(mode, act, sys_shift, acc_shift, relu6_shift)        \
	ROCC_INSTRUCTION_RS1_RS2(                                              \
		XCUSTOM_ACC,                                                   \
		((uint64_t)(acc_shift) << 32) | ((act) << 3) | ((mode) << 2) | \
			CONFIG_EX,                                             \
		((uint64_t)(relu6_shift) << 32) | (sys_shift), k_CONFIG)

#define gemmini_config_ld(stride) \
	ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, CONFIG_LD, stride, k_CONFIG)

#define gemmini_config_st(stride) \
	ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, CONFIG_ST, stride, k_CONFIG)

// flush
#define gemmini_flush(skip) \
	ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, skip, 0, k_FLUSH)

// fence
#define gemmini_fence() asm volatile("fence")

// Tiling functions
void sp_tiled_matmul_os(const elem_t *A, const elem_t *B, const acc_t *D,
			       elem_t *C, size_t I, size_t J, size_t K,
			       size_t pad_I, size_t pad_J, size_t pad_K,
			       size_t A_row_len, size_t B_row_len,
			       size_t D_row_len, size_t C_row_len, bool no_bias,
			       bool repeating_bias);

void sp_tiled_matmul_ws(const elem_t *A, const elem_t *B, const acc_t *D,
			       elem_t *C, size_t I, size_t J, size_t K,
			       size_t pad_I, size_t pad_J, size_t pad_K,
			       size_t A_row_len, size_t B_row_len,
			       size_t D_row_len, size_t C_row_len, bool no_bias,
			       bool repeating_bias);

void tiled_matmul_outer(size_t dim_I, size_t dim_J, size_t dim_K,
			       const elem_t A[dim_I][dim_K],
			       const elem_t B[dim_K][dim_J], const acc_t *D,
			       elem_t C[dim_I][dim_J], size_t tile_I,
			       size_t tile_J, size_t tile_K, int act, int shift,
			       size_t relu6_shift, bool repeating_bias,
			       int dataflow);

/*
void tiled_matmul_ws(size_t dim_I, size_t dim_J, size_t dim_K,
        const elem_t A[dim_I][dim_K], const elem_t B[dim_K][dim_J],
        const acc_t * D, elem_t C[dim_I][dim_J],
        size_t tile_I, size_t tile_J, size_t tile_K,
        int act, size_t shift, size_t relu6_shift, bool repeating_bias);
*/

void matmul_cpu(size_t dim_I, size_t dim_J, size_t dim_K,
		       const elem_t A[dim_I][dim_K],
		       const elem_t B[dim_K][dim_J], const acc_t *D,
		       elem_t C[dim_I][dim_J], int act, size_t shift,
		       size_t relu6_shift, bool repeating_bias);

/*
void matmul_cpu(size_t DIM_I, size_t DIM_J, size_t DIM_K,
        // elem_t A[DIM_I][DIM_K], elem_t B[DIM_K][DIM_J], acc_t D[DIM_I][DIM_J],
        elem_t A[DIM_I][DIM_K], elem_t B[DIM_K][DIM_J], void * D,
        elem_t C[DIM_I][DIM_J],
        int act, int shift, int relu6_shift, int full_bias_width);
*/

// General matmul which can be run with different dataflows, or on the CPU
enum tiled_matmul_type_t { OS, WS, CPU };

// This function runs a tiled matrix multiplication, with hardcoded tiling
// factors
void tiled_matmul(size_t dim_I, size_t dim_J, size_t dim_K,
		  const elem_t A[dim_I][dim_K], const elem_t B[dim_K][dim_J],
		  const acc_t *D, elem_t C[dim_I][dim_J], int act, size_t shift,
		  size_t relu6_shift, bool repeating_bias, size_t tile_I,
		  size_t tile_J, size_t tile_K,
		  enum tiled_matmul_type_t tiled_matmul_type);

// This function runs a tiled matrix multiplication, with automatically
// calculated tiling factors
void tiled_matmul_auto(size_t dim_I, size_t dim_J, size_t dim_K,
		       const elem_t A[dim_I][dim_K],
		       const elem_t B[dim_K][dim_J], const acc_t *D,
		       elem_t C[dim_I][dim_J], int act, size_t shift,
		       size_t relu6_shift, bool repeating_bias,
		       enum tiled_matmul_type_t tiled_matmul_type);

#endif // SRC_MAIN_C_GEMMINI_H

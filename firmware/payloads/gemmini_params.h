#ifndef GEMMINI_PARAMS_H
#define GEMMINI_PARAMS_H

#include <limits.h>
#include <stdint.h>

#define DIM 16
#define ADDR_LEN 32
#define BANK_NUM 4
#define BANK_ROWS 4096
#define ACC_ROWS 1024
#define MAX_BYTES 64
#define MAX_BLOCK_LEN (MAX_BYTES / (DIM * 1))
#define MAX_BLOCK_LEN_ACC (MAX_BYTES / (DIM * 4))

typedef int8_t elem_t;
#define elem_t_max (elem_t)(127)
#define elem_t_min (elem_t)(-128)
typedef int32_t acc_t;
typedef int64_t full_t;

#define row_align(blocks) \
	__attribute__((aligned(blocks * DIM * sizeof(elem_t))))
#define row_align_acc(blocks) \
	__attribute__((aligned(blocks * DIM * sizeof(acc_t))))

#endif // GEMMINI_PARAMS_H

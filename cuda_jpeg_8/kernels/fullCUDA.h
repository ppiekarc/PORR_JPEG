#pragma once

#if defined __cplusplus
extern "C"
#endif

#include <stdint.h>

int16_t *full_dct_CUDAv2(uint8_t *R, uint8_t *G, uint8_t *B, size_t width, size_t height, int *num_blocks, const float *dtY, const float *dtCb,
                          int32_t *tYR, int32_t *tYG, int32_t *tYB, int32_t *tCbR, int32_t *tCbG, int32_t *tCbB,
                         int32_t *tCrR, int32_t *tCrG, int32_t *tCrB);

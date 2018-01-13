#pragma once

#if defined __cplusplus
extern "C"
#endif

#include <stdint.h>

int16_t *dct_CUDA(int8_t *Y, int8_t *Cb, int8_t *Cr, size_t width, size_t height, int *num_blocks, const float *dtY, const float *dtCb);

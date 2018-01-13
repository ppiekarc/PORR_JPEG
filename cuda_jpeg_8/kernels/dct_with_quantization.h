#pragma once

#if defined __cplusplus
extern "C"
#endif

#include <stdint.h>

int16_t *dct_with_quantization(int8_t *Y_in, int8_t *Cb_in, int8_t *Cr_in, size_t width, size_t height, int *num_blocks,
                               const float *dtY, const float *dtCb);

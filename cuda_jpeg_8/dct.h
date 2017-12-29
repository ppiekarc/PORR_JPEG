#pragma once

#include <stdint.h>

int16_t **dct(const int8_t *data_in, size_t width, size_t height, int *num_blocks, const float *dt);
int16_t **dct_naive(const int8_t *const data_in, const size_t width, const size_t height, int *num_blocks, const uint8_t *dt);

#pragma once

#include <stdint.h>

int16_t **dct_for_blocks(const int8_t *const data_in, const size_t width, const size_t height, int *const num_blocks, const float *dt);

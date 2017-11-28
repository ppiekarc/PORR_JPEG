#pragma once

#include <stdint.h>

void dct_for_one_block(unsigned char *data_in, short *data_out);
int16_t **dct_for_blocks(const int8_t *const data_in, const size_t width, const size_t height, int *const num_blocks, const float *dt);

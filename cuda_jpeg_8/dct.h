#pragma once

#include <stdint.h>

int16_t **dct(const int8_t *data_in, size_t width, size_t height, int *num_blocks, const float *dt);

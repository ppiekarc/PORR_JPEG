#include "quantization.h"

static const double aanscalefactor[8] = { 1.0, 1.387039845, 1.306562965, 1.175875602, 1.0, 0.785694958, 0.541196100, 0.275899379};

static const uint8_t zigzag[64]={ 0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42,
                                  3, 8, 12, 17, 25, 30, 41, 43, 9, 11, 18, 24, 31, 40, 44,
                                  53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46,
                                  51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48,
                                  49, 57, 58, 62, 63 };

const float *const prepare_quantization_table(const uint8_t *const table) {
    float *const fd_table = malloc(sizeof(float) * 64);

    size_t i = 0;
    for (size_t row = 0; row < 8; row++) {
        for (size_t col = 0; col < 8; col++) {
            *(fd_table + i) = (float) (1.0 / ((double) table[zigzag[i]] * aanscalefactor[row] * aanscalefactor[col] * 8.0));
            i++;
        }
    }

    return fd_table;
}

void scale_quantization_table_with_zigzag(const uint8_t *const input_table, const uint8_t scale_factor, uint8_t *const scaled_table)
{
    for (size_t i = 0; i < 64; i++) {
        long temp = ((long) input_table[i] * scale_factor + 50L) / 100L;
        /* limit the values to the valid range */
        if (temp <= 0L) temp = 1L;
        if (temp > 255L) temp = 255L;
        scaled_table[zigzag[i]] = (uint8_t) temp;
    }
}

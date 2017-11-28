#include <stdio.h>
#include "timer.h"
#include "dct.h"
#include "huffman.h"
#include "bmp_loader.h"
#include "ycc_converter.h"
#include "quantization.h"
#include "math.h"

static const uint8_t QUANTIZATION_TABLE_SCALE_FACTOR = 50;
static const int number_of_dct_blocks;
static channel_encoding_context yctx = {0};
static channel_encoding_context cbctx = {0};
static channel_encoding_context crctx = {0};

int main(int argc, char *argv[])
{
    init_Huffman_tables();

    const ImageRGB *const rgb_image = load_true_rgb_bitmap("../resources/lena.bmp");
    const size_t image_size = rgb_image->height * rgb_image->width;
    yctx.encoded_channel = malloc(image_size);
    cbctx.encoded_channel = malloc(image_size);
    crctx.encoded_channel = malloc(image_size);

    const size_t num_blocks = (int)roundf((image_size) / 64);
    yctx.encoded_dct_indices = malloc(sizeof(size_t) * num_blocks);
    cbctx.encoded_dct_indices = malloc(sizeof(size_t) * num_blocks);
    crctx.encoded_dct_indices = malloc(sizeof(size_t) * num_blocks);

    static uint8_t chrominance_quantization_table[64] = {};
    static uint8_t luminance_quantization_table[64] = {};
    scale_quantization_table_with_zigzag(std_luminance_qt, QUANTIZATION_TABLE_SCALE_FACTOR, luminance_quantization_table);
    scale_quantization_table_with_zigzag(std_chrominance_qt, QUANTIZATION_TABLE_SCALE_FACTOR, chrominance_quantization_table);

    const float *const fdtbl_Y = prepare_quantization_table(luminance_quantization_table);
    const float *const fdtbl_Cb = prepare_quantization_table(chrominance_quantization_table);


    const size_t NUM_OF_COMPRESSIONS = 1000;
    double *time_per_each_compression = malloc(sizeof(double) * NUM_OF_COMPRESSIONS);

    app_timer_t start, stop;

    for (size_t i = 0; i < NUM_OF_COMPRESSIONS; i++) {
        timer(&start);

        const ImageYCC *const image = convert(rgb_image);

        int16_t **y_data_out = dct_for_blocks(image->Y, image->width, image->height, &number_of_dct_blocks, fdtbl_Y);
        int16_t **cb_data_out = dct_for_blocks(image->Cb, image->width, image->height, &number_of_dct_blocks, fdtbl_Cb);
        int16_t **cr_data_out = dct_for_blocks(image->Cr, image->width, image->height, &number_of_dct_blocks, fdtbl_Cb);

        for (size_t block = 0; block < number_of_dct_blocks; block++) {
            encode_block(y_data_out[block], YDC_HT, YAC_HT, &yctx, block);
            encode_block(cb_data_out[block], CbDC_HT, CbAC_HT, &cbctx, block);
            encode_block(cr_data_out[block], CbDC_HT, CbAC_HT, &crctx, block);
        }

        timer(&stop);
        *(time_per_each_compression + i) = elapsed_time(start, stop);
    }

    double sum = 0.0;
    for (size_t i = 0; i < NUM_OF_COMPRESSIONS; i++)
        sum += time_per_each_compression[i];

    printf("CPU (total!) time = %.3f ms\n", (sum / NUM_OF_COMPRESSIONS));

    release(rgb_image);
    free(time_per_each_compression);

#if defined(_WIN32) || defined(_WIN64)
    system("PAUSE");
#endif

    return 0;
}

#include <stdio.h>
#include "dct.h"
#include "huffman.h"
#include "bmp_loader.h"
#include "ycc_converter.h"
#include "quantization.h"
#include "math.h"
#include "timer.h"
#include <pthread.h>

static const uint8_t QUANTIZATION_TABLE_SCALE_FACTOR = 50;

typedef struct {
    size_t width, height;
    uint8_t *R;
    uint8_t *G;
    uint8_t *B;
    float *fd_luminance;
    float *fd_chrominance;
} image_block_thread_data;

typedef struct {
    int16_t **y_dct;
    int16_t **cb_dct;
    int16_t **cr_dct;
    size_t num_of_blocks;
} thread_conversion_result;


thread_conversion_result *ycc_conversion_with_dct_thread_func(void *arg) {
    const int number_of_dct_blocks;
    image_block_thread_data *thread_data = (image_block_thread_data *)arg;

    const ImageRGB rgb_image = {
            .height = thread_data->height,
            .width = thread_data->width,
            .R = thread_data->R,
            .B = thread_data->B,
            .G = thread_data->G
    };

    thread_conversion_result *conversion_result = malloc(sizeof(thread_conversion_result));

    const ImageYCC *const image = convertImage(&rgb_image);
    conversion_result->y_dct = dct(image->Y, thread_data->width, thread_data->height, &number_of_dct_blocks, thread_data->fd_luminance);
    conversion_result->cr_dct = dct(image->Cr, thread_data->width, thread_data->height, &number_of_dct_blocks, thread_data->fd_chrominance);
    conversion_result->cb_dct = dct(image->Cb, thread_data->width, thread_data->height, &number_of_dct_blocks, thread_data->fd_chrominance);
    conversion_result->num_of_blocks = number_of_dct_blocks;
    return conversion_result;
}

static channel_encoding_context yctx = {0};
static channel_encoding_context cbctx = {0};
static channel_encoding_context crctx = {0};

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s <input_bitmap> [nr_of_compressions]\n", argv[0]);
        return 0;
    }

    size_t NUM_OF_COMPRESSIONS = 1000;

    if (argc == 3 && atoi(argv[2]) > 0) {
        NUM_OF_COMPRESSIONS = atoi(argv[2]);
    }

    const char *const bitmap_filename = argv[1];
    const ImageRGB *const rgb_image = load_true_rgb_bitmap(bitmap_filename);
    printf("Benchmarking with %s using total of %i compressions...\n", bitmap_filename, NUM_OF_COMPRESSIONS);

    init_Huffman_tables();

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

    double *time_per_each_compression = malloc(sizeof(double) * NUM_OF_COMPRESSIONS);

    pthread_t thread1, thread2, thread3, thread4;
    pthread_t thread5, thread6, thread7, thread8;

    const size_t block_size = (rgb_image->width * rgb_image->height) / 8;

    image_block_thread_data block_thread_data1 = {
            .height = rgb_image->height / 8,
            .width = rgb_image->width,
            .fd_luminance = fdtbl_Y,
            .fd_chrominance = fdtbl_Cb,
            .R = rgb_image->R,
            .G = rgb_image->G,
            .B = rgb_image->B
    };

    image_block_thread_data block_thread_data2 = {
            .height = rgb_image->height / 8,
            .width = rgb_image->width,
            .fd_luminance = fdtbl_Y,
            .fd_chrominance = fdtbl_Cb,
            .R = rgb_image->R + block_size,
            .G = rgb_image->G + block_size,
            .B = rgb_image->B + block_size
    };

    image_block_thread_data block_thread_data3 = {
            .height = rgb_image->height / 8,
            .width = rgb_image->width,
            .fd_luminance = fdtbl_Y,
            .fd_chrominance = fdtbl_Cb,
            .R = rgb_image->R + 2 * block_size,
            .G = rgb_image->G + 2 * block_size,
            .B = rgb_image->B + 2 * block_size
    };

    image_block_thread_data block_thread_data4 = {
            .height = rgb_image->height / 8,
            .width = rgb_image->width,
            .fd_luminance = fdtbl_Y,
            .fd_chrominance = fdtbl_Cb,
            .R = rgb_image->R + 3 * block_size,
            .G = rgb_image->G + 3 * block_size,
            .B = rgb_image->B + 3 * block_size
    };

    image_block_thread_data block_thread_data5 = {
            .height = rgb_image->height / 8,
            .width = rgb_image->width,
            .fd_luminance = fdtbl_Y,
            .fd_chrominance = fdtbl_Cb,
            .R = rgb_image->R + 4 * block_size,
            .G = rgb_image->G + 4 * block_size,
            .B = rgb_image->B + 4 * block_size
    };

    image_block_thread_data block_thread_data6 = {
            .height = rgb_image->height / 8,
            .width = rgb_image->width,
            .fd_luminance = fdtbl_Y,
            .fd_chrominance = fdtbl_Cb,
            .R = rgb_image->R + 5 * block_size,
            .G = rgb_image->G +5 * block_size,
            .B = rgb_image->B + 5 * block_size
    };

    image_block_thread_data block_thread_data7 = {
            .height = rgb_image->height / 8,
            .width = rgb_image->width,
            .fd_luminance = fdtbl_Y,
            .fd_chrominance = fdtbl_Cb,
            .R = rgb_image->R +6 * block_size,
            .G = rgb_image->G + 6 * block_size,
            .B = rgb_image->B + 6 * block_size
    };

    image_block_thread_data block_thread_data8 = {
            .height = rgb_image->height / 8,
            .width = rgb_image->width,
            .fd_luminance = fdtbl_Y,
            .fd_chrominance = fdtbl_Cb,
            .R = rgb_image->R + 7 * block_size,
            .G = rgb_image->G + 7 * block_size,
            .B = rgb_image->B + 7 * block_size
    };

    app_timer_t start, stop;

    thread_conversion_result *conversion_result1, *conversion_result2, *conversion_result3, *conversion_result4;
    thread_conversion_result *conversion_result5, *conversion_result6, *conversion_result7, *conversion_result8;

    for (size_t i = 0; i < NUM_OF_COMPRESSIONS; i++) {
        yctx.current_dc = yctx.encoding_index = 0;
        cbctx.current_dc = cbctx.encoding_index = 0;
        crctx.current_dc = crctx.encoding_index = 0;
        bytepos = 7;

        timer(&start);

        pthread_create(&thread1, NULL, ycc_conversion_with_dct_thread_func, &block_thread_data1);
        pthread_create(&thread2, NULL, ycc_conversion_with_dct_thread_func, &block_thread_data2);
        pthread_create(&thread3, NULL, ycc_conversion_with_dct_thread_func, &block_thread_data3);
        pthread_create(&thread4, NULL, ycc_conversion_with_dct_thread_func, &block_thread_data4);
        pthread_create(&thread5, NULL, ycc_conversion_with_dct_thread_func, &block_thread_data5);
        pthread_create(&thread6, NULL, ycc_conversion_with_dct_thread_func, &block_thread_data6);
        pthread_create(&thread7, NULL, ycc_conversion_with_dct_thread_func, &block_thread_data7);
        pthread_create(&thread8, NULL, ycc_conversion_with_dct_thread_func, &block_thread_data8);

        pthread_join(thread1, &conversion_result1);
        pthread_join(thread2, &conversion_result2);
        pthread_join(thread3, &conversion_result3);
        pthread_join(thread4, &conversion_result4);
        pthread_join(thread5, &conversion_result5);
        pthread_join(thread6, &conversion_result6);
        pthread_join(thread7, &conversion_result7);
        pthread_join(thread8, &conversion_result8);

        const size_t dct_blocks_per_image_block = conversion_result1->num_of_blocks;

        for (size_t block = 0; block < dct_blocks_per_image_block; block++) {
            encode_block(conversion_result1->y_dct[block], YDC_HT, YAC_HT, &yctx, block);
            encode_block(conversion_result1->cb_dct[block], CbDC_HT, CbAC_HT, &cbctx, block);
            encode_block(conversion_result1->cr_dct[block], CbDC_HT, CbAC_HT, &crctx, block);
        }

        for (size_t block = 0; block < dct_blocks_per_image_block; block++) {
            encode_block(conversion_result2->y_dct[block], YDC_HT, YAC_HT, &yctx, block + dct_blocks_per_image_block);
            encode_block(conversion_result2->cb_dct[block], CbDC_HT, CbAC_HT, &cbctx, block + dct_blocks_per_image_block);
            encode_block(conversion_result2->cr_dct[block], CbDC_HT, CbAC_HT, &crctx, block + dct_blocks_per_image_block);
        }

        for (size_t block = 0; block < dct_blocks_per_image_block; block++) {
            encode_block(conversion_result3->y_dct[block], YDC_HT, YAC_HT, &yctx, block + 2 * dct_blocks_per_image_block);
            encode_block(conversion_result3->cb_dct[block], CbDC_HT, CbAC_HT, &cbctx, block + 2 * dct_blocks_per_image_block);
            encode_block(conversion_result3->cr_dct[block], CbDC_HT, CbAC_HT, &crctx, block + 2 * dct_blocks_per_image_block);
        }

        for (size_t block = 0; block < dct_blocks_per_image_block; block++) {
            encode_block(conversion_result4->y_dct[block], YDC_HT, YAC_HT, &yctx, block + 3 * dct_blocks_per_image_block);
            encode_block(conversion_result4->cb_dct[block], CbDC_HT, CbAC_HT, &cbctx, block + 3 * dct_blocks_per_image_block);
            encode_block(conversion_result4->cr_dct[block], CbDC_HT, CbAC_HT, &crctx, block + 3 * dct_blocks_per_image_block);
        }

        for (size_t block = 0; block < dct_blocks_per_image_block; block++) {
            encode_block(conversion_result5->y_dct[block], YDC_HT, YAC_HT, &yctx, block + 4 * dct_blocks_per_image_block);
            encode_block(conversion_result5->cb_dct[block], CbDC_HT, CbAC_HT, &cbctx, block + 4 * dct_blocks_per_image_block);
            encode_block(conversion_result5->cr_dct[block], CbDC_HT, CbAC_HT, &crctx, block + 4 * dct_blocks_per_image_block);
        }

        for (size_t block = 0; block < dct_blocks_per_image_block; block++) {
            encode_block(conversion_result6->y_dct[block], YDC_HT, YAC_HT, &yctx, block + 5 * dct_blocks_per_image_block);
            encode_block(conversion_result6->cb_dct[block], CbDC_HT, CbAC_HT, &cbctx, block + 5 * dct_blocks_per_image_block);
            encode_block(conversion_result6->cr_dct[block], CbDC_HT, CbAC_HT, &crctx, block + 5 * dct_blocks_per_image_block);
        }

        for (size_t block = 0; block < dct_blocks_per_image_block; block++) {
            encode_block(conversion_result7->y_dct[block], YDC_HT, YAC_HT, &yctx, block + 6 * dct_blocks_per_image_block);
            encode_block(conversion_result7->cb_dct[block], CbDC_HT, CbAC_HT, &cbctx, block + 6 * dct_blocks_per_image_block);
            encode_block(conversion_result7->cr_dct[block], CbDC_HT, CbAC_HT, &crctx, block + 6 * dct_blocks_per_image_block);
        }

        for (size_t block = 0; block < dct_blocks_per_image_block; block++) {
            encode_block(conversion_result8->y_dct[block], YDC_HT, YAC_HT, &yctx, block + 7 * dct_blocks_per_image_block);
            encode_block(conversion_result8->cb_dct[block], CbDC_HT, CbAC_HT, &cbctx, block + 7 * dct_blocks_per_image_block);
            encode_block(conversion_result8->cr_dct[block], CbDC_HT, CbAC_HT, &crctx, block + 7 * dct_blocks_per_image_block);
        }

        timer(&stop);
        *(time_per_each_compression + i) = elapsed_time(start, stop);
    }

    double sum = 0.0;
    for (size_t i = 0; i < NUM_OF_COMPRESSIONS; i++)
        sum += (time_per_each_compression[i] / NUM_OF_COMPRESSIONS);

    printf("CPU (total!) time = %.3f ms\n", sum);

#if defined(_WIN32) || defined(_WIN64)
    system("PAUSE");
#endif

    return 0;
}
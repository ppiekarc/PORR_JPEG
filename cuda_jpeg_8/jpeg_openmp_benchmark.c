#include <stdio.h>
#include <memory.h>
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

	app_timer_t start, stop;

	for (size_t i = 0; i < NUM_OF_COMPRESSIONS; i++) {
		yctx.current_dc = yctx.encoding_index = 0;
		cbctx.current_dc = cbctx.encoding_index = 0;
		crctx.current_dc = crctx.encoding_index = 0;
		bytepos = 7;

		timer(&start);

		const ImageYCC *const image = convertImage(rgb_image);

		int16_t **y_data_out = dct(image->Y, image->width, image->height, &number_of_dct_blocks, fdtbl_Y);
		int16_t **cb_data_out = dct(image->Cb, image->width, image->height, &number_of_dct_blocks, fdtbl_Cb);
		int16_t **cr_data_out = dct(image->Cr, image->width, image->height, &number_of_dct_blocks, fdtbl_Cb);

		for (size_t block = 0; block < num_blocks; block++) {
			encode_block(y_data_out[block], YDC_HT, YAC_HT, &yctx, block);
			encode_block(cb_data_out[block], CbDC_HT, CbAC_HT, &cbctx, block);
			encode_block(cr_data_out[block], CbDC_HT, CbAC_HT, &crctx, block);
		}

		timer(&stop);
		*(time_per_each_compression + i) = elapsed_time(start, stop);

		free(y_data_out);
		free(cr_data_out);
		free(cb_data_out);
		free(image->Y);
		free(image->Cb);
		free(image->Cr);
		free(image);
	}

	double sum = 0.0;
	for (size_t i = 0; i < NUM_OF_COMPRESSIONS; i++)
		sum += time_per_each_compression[i];

	printf("CPU (total!) time = %.3f ms\n", (sum / NUM_OF_COMPRESSIONS));

#if defined(_WIN32) || defined(_WIN64)
	system("PAUSE");
#endif

	return 0;
}
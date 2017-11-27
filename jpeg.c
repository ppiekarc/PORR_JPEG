#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include "ycc_converter.h"
#include "timer.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

struct AC_Symbol {
	int runlenght;
	int size;
	short amplitude;
};


struct AC_Symbols {
	struct AC_Symbol *block;
	int size;
};

/* That define means we use 8x8 block of discrete cosinus transform*/
#define K 8

/* This is a test image from: https://en.wikipedia.org/wiki/JPEG*/
static unsigned char example[64] = 
{
	52, 55, 61, 66, 70, 61, 64, 73,
	63, 59, 55, 90, 109, 85, 69, 72,
	62, 59, 68, 113, 144, 104, 66, 73,
	63, 58, 71, 122, 154, 106, 70, 69,
	67, 61, 68, 104, 126, 88, 68, 70,
	79, 65, 60, 70, 77, 68, 58, 75,
	85, 71, 64, 59, 55, 61, 65, 83,
	87, 79, 69, 68, 65, 76, 78, 94
};

/* This will contain enter test data*/
static unsigned char test1[4 * 64];

/*This function prepare test data, which contain 4 blocks 8x8 (example []) 
 (now only for one color), with it we could test divide to blocks		*/
void prepare_test1(void)
{
	int i, j;

	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			test1[i + 16 * j] = example[i + 8 * j];
			test1[(i + 8) + 16 * j] = example[i + 8 * j];
			test1[(i + 128) + 16 * j] = example[i + 8 * j];
			test1[(i + 128 + 8) + 16 * j] = example[i + 8 * j];
		}
	}

}

/*wstep do kodowania skladowej zmiennej
	dla jednego koloru	*/
struct AC_Symbols ac_block_code(short *ac)
{
	int runlenght = 0, size = 0, i;
	struct AC_Symbols symbols_ac;


	struct AC_Symbol *ac_block = malloc((K * K - 1) * sizeof(struct AC_Symbol));

		for (i = 1; i < 64; i++) {
			if (runlenght == 16) {
				ac_block[size].runlenght = 15;
				ac_block[size].amplitude = 0;
				ac_block[size].size = 0;
				runlenght = 0;
				size++;
				continue;
			}

			if (ac[i] == 0)
				runlenght++;
			else {
				ac_block[size].runlenght = runlenght;
				ac_block[size].amplitude = ac[i];
				ac_block[size].size = ceil(log2(abs(ac[i]) + 1));
				runlenght = 0;
				size++;
			}
		}
	ac_block[size].runlenght = 0;
	ac_block[size].size = 0;
	ac_block[size].amplitude = 0;
	size++;

	symbols_ac.block = ac_block;
	symbols_ac.size = size;

	return symbols_ac;
}

/* wtep do kodowanie skaldowej stalej
	dla jednego koloru*/
short *dc_code(short **data, int num_blocks)
{
	short *delta = malloc(num_blocks * sizeof(short));

	delta[0] = data[0][0];

	for (int i = 1; i < num_blocks; i++) 
		delta[i] = data[i][0] - data[i - 1][0];

	return delta;
}

void test_fun()
{
	int i, number_of_blocks;
	struct AC_Symbols *symbols;
	short *dc;
	short **data_out;

	prepare_test1();
//	data_out = dct_for_blocks(test1, 16, 16, &number_of_blocks);
	symbols = malloc(number_of_blocks * sizeof(struct AC_Symbols));

	dc = dc_code(data_out, number_of_blocks);
	for (int i = 0; i < number_of_blocks; i++)
		symbols[i] = ac_block_code(data_out[i]);


	printf("skladowa stala: \n");
	for (i = 0; i < number_of_blocks; i++)
		printf(" %d ", dc[i]);


	printf("skladowa zmienna: \n");
	for (i = 0; i < symbols[0].size; i++) {
		printf("(%d, %d) (%d) ",
			symbols[0].block[i].runlenght,
			symbols[0].block[i].size,
			symbols[0].block[i].amplitude);
	}

	for (int i = 0; i < number_of_blocks; i++) {
		free(symbols[i].block);
		free(data_out[i]);
	}

	free(symbols);
	free(dc);
	free(data_out);
}

void test_with_image()
{
	int i, number_of_blocks;
	struct AC_Symbols *symbols;
	short *dc;
	short **data_out;
	ImageYCC *image;

//	image = load_true_rgb_bitmap("../resources/tahaa.bmp");
	/* obliczenie transforamy cosinusowej razem z podzialem na bloki*/
//	data_out = dct_for_blocks(image->R, image->width, image->height, &number_of_blocks);


	symbols = malloc(number_of_blocks * sizeof(struct AC_Symbols));

	/* wstep do kodowania skladowych stalych i zmiennych*/
	dc = dc_code(data_out, number_of_blocks);


#pragma omp parallel private(i) shared(symbols) num_threads(4)
	{
#pragma omp for schedule (static)
		for (i = 0; i < number_of_blocks; i++)
			symbols[i] = ac_block_code(data_out[i]);
	}


	/*zwolnienie pamieci*/
	for (i = 0; i < number_of_blocks; i++) {
		free(symbols[i].block);
		free(data_out[i]);
	}
	free(symbols);
	free(dc);
	free(data_out);
	free(image->Y);
	free(image->Cb);
	free(image->Cr);
	free(image);
}

int main(int argc, char *argv[])
{
	app_timer_t start, stop;
	timer(&start);
	//test_fun();
//	test_with_image();
//	test_with_grayscale();
	test_with_rgb();
	timer(&stop);
	elapsed_time(start, stop);

#if defined(_WIN32) || defined(_WIN64)
	system("PAUSE");
#endif

	return 0;
}


#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dct.h"

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

struct Image_rgb {
	unsigned char *R;
	unsigned char *B;
	unsigned char *G;
	int width;
	int height;
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


#ifdef _WIN32

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

typedef LARGE_INTEGER app_timer_t;

#define timer(t_ptr) QueryPerformanceCounter(t_ptr)

void elapsed_time(app_timer_t start, app_timer_t stop)
{
	double etime;
	LARGE_INTEGER clk_freq;
	QueryPerformanceFrequency(&clk_freq);
	etime = (stop.QuadPart - start.QuadPart) /
		(double)clk_freq.QuadPart;
	printf("CPU (total!) time = %.3f ms )\n",
		etime * 1e3);
}

#else

#include <time.h> /* requires linking with rt library
(command line option -lrt) */

typedef struct timespec app_timer_t;

#define timer(t_ptr) clock_gettime(CLOCK_MONOTONIC, t_ptr)

void elapsed_time(app_timer_t start, app_timer_t stop)
{
	double etime;
	etime = 1e+3 * (stop.tv_sec - start.tv_sec) +
		1e-6 * (stop.tv_nsec - start.tv_nsec);
	printf("CPU (total!) time = %.3f ms (%6.3f GFLOP/s)\n",
		etime, 1e-6 * flop / etime);
}

#endif


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


struct Image_rgb read_bmp_file(char *filename)
{
	int i;
	FILE *f = fopen(filename, "rb");
	unsigned char info[54];
	unsigned char *data;
	struct Image_rgb image;

	fread(info, sizeof(unsigned char), 54, f);  // read the 54-byte header

	/* extract image height and width from header */
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	int s;
	int size = width * height;
	data = (unsigned char *)malloc(3 * size * sizeof(unsigned char)); // allocate 3 bytes per pixel
	s = fread(data, sizeof(unsigned char), 3 * size, f); // read the rest of the data at once'

	fclose(f);

	image.width = width;
	image.height = height;
	image.R = (unsigned char *)malloc(size * sizeof(unsigned char));
	image.B = (unsigned char *)malloc(size * sizeof(unsigned char));
	image.G = (unsigned char *)malloc(size * sizeof(unsigned char));

	for (i = 0; i < (3 * size); i += 3)
	{
		image.B[i / 3] = data[i];
		image.G[i / 3] = data[i + 1];
		image.R[i / 3] = data[i + 2];
	}

	free(data);
	/* Now data should contain the(R, G,B) values of the pixels.

	/*In the last part, the swap between every first and third pixel is done
	beceouse windows stores the color values as (B, G, R) triples not (R, G, B)*/
	return image;
}

/*wstep do kodowania skladowej zmiennej
	dla jednego koloru	*/
struct AC_Symbols ac_block_code(short *ac)
{
	int runlenght = 0, size = 0;
	struct AC_Symbols symbols_ac;


	struct AC_Symbol *ac_block = malloc((K * K - 1) * sizeof(struct AC_Symbol));
	for (int i = 1; i < 64; i++) {
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

int main(int argc, char *argv[])
{
	int i, number_of_blocks;
	struct AC_Symbols *symbols;
	short *dc;
	app_timer_t start, stop;


	prepare_test1();

	timer(&start);

	short **data_out;
	data_out = dct_for_blocks(test1, 16, 16, &number_of_blocks);
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

	timer(&stop);

	elapsed_time(start, stop);

	free(symbols);
	free(dc);
	free(data_out);

#if defined(_WIN32) || defined(_WIN64)
	system("PAUSE");
#endif

	return 0;
}


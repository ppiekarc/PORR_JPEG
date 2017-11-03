#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif

#pragma once

#include <stdio.h>
#include <windows.h>
#include <math.h>
#include "dct.h"


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
static unsigned char test1[3 * 64];

/*This function write value of example to test1 that contains the same value on Y, Cb and Cr*/
void prepare_test1(void)
{
	int i, j;

	for (i = 0; i < 64; i++) {
		for (j = 0; j < 3; j++)
			test1[3 * i + j] = example[i];
	}
}


/*wstep do kodowania skladowej zmiennej*/
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

int main()
{
	int i;
	struct AC_Symbols symbols;

	prepare_test1();

	short *data_out = malloc(64 * sizeof(short));
	dct_for_one_block(example, data_out);
	symbols = ac_block_code(data_out);

	//for (i = 0; i < 64; i++)
	//	printf(" %d ", data_out[i]);

	for (i = 0; i < symbols.size; i++) {
		printf("(%d, %d) (%d) ", 
			symbols.block[i].runlenght, 
			symbols.block[i].size, 
			symbols.block[i].amplitude);

	}


	free(symbols.block);
	free(data_out);
	system("PAUSE");
}




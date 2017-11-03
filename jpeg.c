#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif

#pragma once

#include <stdio.h>
#include <windows.h>
#include "dct.h"


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



int main()
{
	int i;

	prepare_test1();

	short *data_out = malloc(64 * sizeof(short));
	dct_for_one_block(example, data_out);

	for (i = 0; i < 64; i++)
		printf(" %d ", data_out[i]);

	free(data_out);
	system("PAUSE");
}




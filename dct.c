#include <stdio.h>
#include <math.h>

/* That define means we use 8x8 block of discrete cosinus transform*/
#define K 8

/* This is a qantization matrix for a quality of 50%
as specified in the original JPEG Standard	*/
static int coeff[K * K] =
{
	16, 11, 10, 16, 24, 40, 51, 61,
	12, 12, 14, 19, 26, 58, 60, 55,
	14, 13, 16, 24, 40, 57, 69, 56,
	14, 17, 22, 29, 51, 87, 80, 62,
	18, 22, 37, 56, 68, 109, 103, 77,
	24, 35, 55, 64, 81, 104, 113, 92,
	49, 64, 78, 87, 103, 121, 120, 101,
	72, 92, 95, 98, 112, 100, 103, 99
};


static float cos_table[8][8] = 
{
	{ 1.0000,    0.9808,    0.9239,    0.8315,    0.7071,    0.5556,    0.3827,    0.1951 },
	{ 1.0000,    0.8315,    0.3827, -0.1951, -0.7071, -0.9808, -0.9239, -0.5556 },
	{ 1.0000    ,0.5556 ,-0.3827 ,-0.9808 ,-0.7071,    0.1951,    0.9239,    0.8315 },
	{ 1.0000,    0.1951, -0.9239, -0.5556,    0.7071,    0.8315, -0.3827, -0.9808 },
	{ 1.0000, -0.1951, -0.9239,    0.5556,    0.7071, -0.8315, -0.3827,    0.9808 },
	{ 1.0000, -0.5556, -0.3827,    0.9808, -0.7071, -0.1951,    0.9239, -0.8315 },
	{ 1.0000, -0.8315,    0.3827,    0.1951, -0.7071,    0.9808, -0.9239,    0.5556 },
	{ 1.0000, -0.9808,    0.9239, -0.8315,    0.7071, -0.5556,    0.3827, -0.1951 }
};


static int zigzag_map8x8[8 * 8] = 
{
	0, 1, 5, 6, 14, 15, 27, 28,
	2, 4, 7, 13, 16, 26, 29, 42,
	3, 8, 12, 17, 25, 30, 41, 43,
	9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};

static inline float alfa(int u)
{
	if (u == 0)
		return 0.7071;
	else
		return 1;
}


/* This function compute only one block 8x8 for one color in pixel
   This also comupte quatization and save outup in zigzag order
*/
void dct_for_one_block(unsigned char *data_in, short *data_out)
{
	int u, v, i, j;
	float G;
	short converted_value;


	for (u = 0; u < 8; u++) {
		for (v = 0; v < 8; v++) {
			G = 0;
			for (i = 0; i < 8; i++) {
				for (j = 0; j < 8; j++) {
					converted_value = data_in[i + (8 * j)] - 128;
					G += converted_value * cos_table[i][u] * cos_table[j][v];
				}
			}
			G = G * alfa(u) * alfa(v);
			int index_zigzag = zigzag_map8x8[u + (v * 8)];
			data_out[index_zigzag] = roundf(G / (4 * coeff[u + (v * 8)]));
		}
	}

}



/* Zwracana warosc przez funkje zawiera wskanik na tablice zawierajaca 
   wynik transformaty cosinusowej dla kazdego bloku. W tej funkcji alokowana 
   jest pamiec na kazdy blok, dlatego nalezy zwolnic pamiec ze wszystkich elementow
   zwracaniej tablicy, a nastepnie zwolnic zwracana tablice.
   
   Funckja oblicza transformaty dla jednego koloru.
   */
short **dct_for_blocks(unsigned char *data_in, int w, int h, int *num_blocks)
{
	*num_blocks = (int)roundf((w * h) / 64);
	unsigned char **d_in = malloc(*num_blocks * sizeof(unsigned char *));
	short **data_out = malloc(*num_blocks * sizeof(short *));


	/* alokacja pamieci dla kazdego bloku,
	   przed zakonczeniem programu nalezy ja zwolnic
		(nie jest to zwalnianie w tej funkcji)*/
	for (int i = 0; i < *num_blocks; i++) {
		data_out[i] = malloc(64 * sizeof(short));
		d_in[i] = malloc(64 * sizeof(unsigned char));
	}
		
	
	/* @TODO Podzial prawdopodonie nie dziala jeszcze dla obrazkow,
	   które nie sa dokladnie wielokrotnosciami wielkosci bloku 8x8*/

	/* Ta petla realizuje podzial na bloki, 
		kazdy blok jest elementem tablicy d_in*/
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			for (int n = 0; n < *num_blocks; n++)
				d_in[n][i + (j * 8)] = data_in[i + (n * 8) + (w * j)];
		}
	}

	/* obliczenie transformaty cosinusowej dla kazdego bloku*/
	for (int i = 0; i < *num_blocks; i++) {
		dct_for_one_block(d_in[i], data_out[i]);
		free(d_in[i]);
	}

	return data_out;
}


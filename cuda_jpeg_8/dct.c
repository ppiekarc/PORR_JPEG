#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <stdint.h>

#define alpha(u) ((u == 0) ? 1 / sqrt(8.0f) : 0.5f)

static uint8_t zigzag[64]={ 0, 1, 5, 6,14,15,27,28,
						 2, 4, 7,13,16,26,29,42,
						 3, 8,12,17,25,30,41,43,
						 9,11,18,24,31,40,44,53,
						 10,19,23,32,39,45,52,54,
						 20,22,33,38,46,51,55,60,
						 21,34,37,47,50,56,59,61,
						 35,36,48,49,57,58,62,63 };

static float cos_table[8][8] = {
	{ 1.0000, 0.9808, 0.9239, 0.8315, 0.7071, 0.5556, 0.3827, 0.1951 },
	{ 1.0000, 0.8315, 0.3827, -0.1951, -0.7071, -0.9808, -0.9239, -0.5556 },
	{ 1.0000, 0.5556 ,-0.3827 ,-0.9808 ,-0.7071, 0.1951, 0.9239, 0.8315 },
	{ 1.0000, 0.1951, -0.9239, -0.5556, 0.7071, 0.8315, -0.3827, -0.9808 },
	{ 1.0000, -0.1951, -0.9239, 0.5556, 0.7071, -0.8315, -0.3827, 0.9808 },
	{ 1.0000, -0.5556, -0.3827, 0.9808, -0.7071, -0.1951, 0.9239, -0.8315 },
	{ 1.0000, -0.8315, 0.3827, 0.1951, -0.7071, 0.9808, -0.9239, 0.5556 },
	{ 1.0000, -0.9808, 0.9239, -0.8315, 0.7071, -0.5556, 0.3827, -0.1951 }
};

static void load_8x8_block_at(const int8_t *const pInt, const size_t xpos, const size_t ypos, const size_t width, int8_t *p_dest) {
	size_t pos = 0;
	size_t block_location = ypos * width + xpos;
	for (size_t y = 0; y < 8; y++) {
		for (size_t x = 0; x < 8; x++) {
			p_dest[pos] = pInt[block_location];
			block_location++;
			pos++;
		}
		block_location += width - 8;
	}
}

static void dct_for_one_block_naive(int8_t *data, uint8_t *fdtbl, int16_t *outdata)
{
	int u, v, i, j;
	float G;
	short converted_value;
	
		
		for (u = 0; u < 8; u++) {
		for (v = 0; v < 8; v++) {
			G = 0;
			for (i = 0; i < 8; i++) {
				for (j = 0; j < 8; j++) {
					converted_value = data[i + (8 * j)];
					G += converted_value * cos_table[i][u] * cos_table[j][v];
					
				}
				
			}
			
			G = (((alpha(i)) * (alpha(j))) * G) / fdtbl[u + (v * 8)];
			int index_zigzag = zigzag[u + (v * 8)];
			outdata[index_zigzag] = (int16_t)((int16_t)(G + 16384.5) - 16384);
			
		}
	}

}

static void dct_for_one_block(int8_t *data, float *fdtbl, int16_t *outdata)
{
	float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	float tmp10, tmp11, tmp12, tmp13;
	float z1, z2, z3, z4, z5, z11, z13;
	float *dataptr;
	float datafloat[64];
	float temp;
	int8_t ctr;
	for (size_t i = 0; i < 64; i++) datafloat[i] = data[i];

	/* Pass 1: process rows. */
	dataptr = datafloat;
	for (ctr = 7; ctr >= 0; ctr--) {
		tmp0 = dataptr[0] + dataptr[7];
		tmp7 = dataptr[0] - dataptr[7];
		tmp1 = dataptr[1] + dataptr[6];
		tmp6 = dataptr[1] - dataptr[6];
		tmp2 = dataptr[2] + dataptr[5];
		tmp5 = dataptr[2] - dataptr[5];
		tmp3 = dataptr[3] + dataptr[4];
		tmp4 = dataptr[3] - dataptr[4];

		tmp10 = tmp0 + tmp3;
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;

		dataptr[0] = tmp10 + tmp11;
		dataptr[4] = tmp10 - tmp11;

		z1 = (tmp12 + tmp13) * ((float) 0.707106781);
		dataptr[2] = tmp13 + z1;
		dataptr[6] = tmp13 - z1;

		tmp10 = tmp4 + tmp5;
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;

		z5 = (tmp10 - tmp12) * ((float) 0.382683433);
		z2 = ((float) 0.541196100) * tmp10 + z5;
		z4 = ((float) 1.306562965) * tmp12 + z5;
		z3 = tmp11 * ((float) 0.707106781);

		z11 = tmp7 + z3;
		z13 = tmp7 - z3;

		dataptr[5] = z13 + z2;
		dataptr[3] = z13 - z2;
		dataptr[1] = z11 + z4;
		dataptr[7] = z11 - z4;

		dataptr += 8;
	}

	/* Pass 2: process columns */
	dataptr = datafloat;
	for (ctr = 7; ctr >= 0; ctr--) {
		tmp0 = dataptr[0] + dataptr[56];
		tmp7 = dataptr[0] - dataptr[56];
		tmp1 = dataptr[8] + dataptr[48];
		tmp6 = dataptr[8] - dataptr[48];
		tmp2 = dataptr[16] + dataptr[40];
		tmp5 = dataptr[16] - dataptr[40];
		tmp3 = dataptr[24] + dataptr[32];
		tmp4 = dataptr[24] - dataptr[32];

		tmp10 = tmp0 + tmp3;
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;

		dataptr[0] = tmp10 + tmp11;
		dataptr[32] = tmp10 - tmp11;

		z1 = (tmp12 + tmp13) * ((float) 0.707106781);
		dataptr[16] = tmp13 + z1;
		dataptr[48] = tmp13 - z1;

		tmp10 = tmp4 + tmp5;
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;

		z5 = (tmp10 - tmp12) * ((float) 0.382683433);
		z2 = ((float) 0.541196100) * tmp10 + z5;
		z4 = ((float) 1.306562965) * tmp12 + z5;
		z3 = tmp11 * ((float) 0.707106781);

		z11 = tmp7 + z3;
		z13 = tmp7 - z3;
		dataptr[40] = z13 + z2;
		dataptr[24] = z13 - z2;
		dataptr[8] = z11 + z4;
		dataptr[56] = z11 - z4;

		dataptr++;
	}

	for (size_t i = 0; i < 64; i++) {
		/* quantization and scaling factor */
		temp = datafloat[i] * fdtbl[i];

		/* Round to nearest integer. */
		outdata[i] = (int16_t) ((int16_t)(temp + 16384.5) - 16384);
	}

    /* Reorder in ZigZag order */
	int16_t *t  = malloc(sizeof(int16_t) * 64);
	memcpy(t, outdata, sizeof(int16_t) * 64);
	for (size_t i = 0; i <= 63; i++) outdata[zigzag[i]] = t[i];
    free(t);
}

int16_t **dct(const int8_t *const data_in, const size_t width, const size_t height, int *num_blocks, const float *dt) {
	*num_blocks = (int)roundf((width * height) / 64);
    int8_t **d_in = malloc(*num_blocks * sizeof(int8_t *));
	int16_t **data_out = malloc(*num_blocks * sizeof(int16_t *));

	for (size_t i = 0; i < *num_blocks ; i++) {
		data_out[i] = calloc(64, sizeof(int16_t));
		d_in[i] = calloc(64, sizeof(int8_t));
	}
	size_t n = 0;
	for (size_t ypos = 0; ypos < height; ypos+=8) {
		for (size_t xpos = 0; xpos < width; xpos+=8) {
			load_8x8_block_at(data_in, xpos, ypos, width, d_in[n++]);
		}
	}
    for (int i = 0; i < *num_blocks; i++) {
	    dct_for_one_block(d_in[i], dt, data_out[i]);
		free(d_in[i]);
    }

	free(d_in);
	return data_out;
}

int16_t **dct_naive(const int8_t *const data_in, const size_t width, const size_t height, int *num_blocks, const uint8_t *dt) {
	*num_blocks = (int)roundf((width * height) / 64);
	int8_t **d_in = malloc(*num_blocks * sizeof(int8_t *));
	int16_t **data_out = malloc(*num_blocks * sizeof(int16_t *));

	for (size_t i = 0; i < *num_blocks; i++) {
		data_out[i] = calloc(64, sizeof(int16_t));
		d_in[i] = calloc(64, sizeof(int8_t));
	}
	size_t n = 0;
	for (size_t ypos = 0; ypos < height; ypos += 8) {
		for (size_t xpos = 0; xpos < width; xpos += 8) {
			load_8x8_block_at(data_in, xpos, ypos, width, d_in[n++]);
		}
	}
	for (int i = 0; i < *num_blocks; i++) {
		dct_for_one_block_naive(d_in[i], dt, data_out[i]);
		free(d_in[i]);
	}

	free(d_in);
	return data_out;
}


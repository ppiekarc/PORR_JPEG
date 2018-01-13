#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../ycc_converter.h"
#include "dctCUDA.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "helper_cuda.h"
#include "helper_timer.h"
#include "../huffman.h"

#define N 64
#define Y 0
#define Cb 1
#define Cr 2

__constant__ static uint8_t zigzag[64] = { 
		0, 1, 5, 6,14,15,27,28,
		2, 4, 7,13,16,26,29,42,
		3, 8,12,17,25,30,41,43,
		9,11,18,24,31,40,44,53,
		10,19,23,32,39,45,52,54,
		20,22,33,38,46,51,55,60,
		21,34,37,47,50,56,59,61,
		35,36,48,49,57,58,62,63 
};

__constant__ static float cos_table[8][8] = {
	{ 1.0000, 0.9808, 0.9239, 0.8315, 0.7071, 0.5556, 0.3827, 0.1951 },
	{ 1.0000, 0.8315, 0.3827, -0.1951, -0.7071, -0.9808, -0.9239, -0.5556 },
	{ 1.0000, 0.5556 ,-0.3827 ,-0.9808 ,-0.7071, 0.1951, 0.9239, 0.8315 },
	{ 1.0000, 0.1951, -0.9239, -0.5556, 0.7071, 0.8315, -0.3827, -0.9808 },
	{ 1.0000, -0.1951, -0.9239, 0.5556, 0.7071, -0.8315, -0.3827, 0.9808 },
	{ 1.0000, -0.5556, -0.3827, 0.9808, -0.7071, -0.1951, 0.9239, -0.8315 },
	{ 1.0000, -0.8315, 0.3827, 0.1951, -0.7071, 0.9808, -0.9239, 0.5556 },
	{ 1.0000, -0.9808, 0.9239, -0.8315, 0.7071, -0.5556, 0.3827, -0.1951 }
};

//__constant__ static uint8_t fdtbl_Y[64] = {
//	16,  11,  10,  16,  24,  40,  51,  61,
//	12,  12,  14,  19,  26,  58,  60,  55,
//	14,  13,  16,  24,  40,  57,  69,  56,
//	14,  17,  22,  29,  51,  87,  80,  62,
//	18,  22,  37,  56,  68, 109, 103,  77,
//	24,  35,  55,  64,  81, 104, 113,  92,
//	49,  64,  78,  87, 103, 121, 120, 101,
//	72,  92,  95,  98, 112, 100, 103,  99
//};
//__constant__ static uint8_t fdtbl_Cb[64] = {
//	17,  18,  24,  47,  99,  99,  99,  99,
//	18,  21,  26,  66,  99,  99,  99,  99,
//	24,  26,  56,  99,  99,  99,  99,  99,
//	47,  66,  99,  99,  99,  99,  99,  99,
//	99,  99,  99,  99,  99,  99,  99,  99,
//	99,  99,  99,  99,  99,  99,  99,  99,
//	99,  99,  99,  99,  99,  99,  99,  99,
//	99,  99,  99,  99,  99,  99,  99,  99
//};

__constant__ static uint8_t fdtbl_Y[64];
__constant__ static uint8_t fdtbl_Cb[64];


#define alpha(u) ((u == 0) ? 1 / sqrt(8.0f) : 0.5f)
#define image_(t, index) image[(t * nr_p) + index]
#define result_(t, b, p) result[(t * nr_p) + (b * 64) + p]

__global__ static void dtf_kernel(int16_t *result, int8_t *image, size_t width, size_t height, size_t nr_p)
{
	__shared__ int8_t block_in[N];
	int16_t converted;
	int i = threadIdx.x;
	int j = threadIdx.y;
	/* w polu type zawarta informacja krorej skladowej dotycza obliczenia : Y Cb Cr */
	int type = blockIdx.z;

	/* wyliczone wartosci indexow tak aby podzielic na odpowiednie bloki, 
		aby kazdy watek wzial odpowiednia dla siebie probke	*/
	int index = (blockIdx.x * blockDim.x) + i + (width * blockIdx.y * blockDim.y) + width * j;

	/* przypisanie odpowiednich wartosci pikseli do pamieci wspoldzielonej przez blok */
	block_in[i + 8 * j] = image_(type, index);

	float Gij = 0;
	/* obliczenie dyskretnej transoframy cosinusowej dla bloku*/
	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++) {
			converted = block_in[x + 8 * y];
			Gij += converted * cos_table[x][i] * cos_table[y][j];

		}
	}

	Gij = ((alpha(i)) * (alpha(j))) * Gij;

	/* kwantyzacja */
	float tmp;

	if (type == Y)
		tmp = Gij / fdtbl_Y[i + 8 * j];
	
	else
		tmp = Gij / fdtbl_Cb[i + 8 * j];


	/* przypisanie wartosci w kolejnosci zigzag */
	size_t block_nr = blockIdx.x + (gridDim.x * blockIdx.y);
	size_t pixel_nr = zigzag[i + 8 * j];
	result_(type, block_nr, pixel_nr) = (int16_t)(tmp);
}


int16_t *dct_CUDA(int8_t *Y_in, int8_t *Cb_in, int8_t *Cr_in, size_t width, size_t height, int *num_blocks, const float *dtY, const float *dtCb)
{
	int8_t *dev_image; /* zawiera 3 skladowe obrazka (Y, Cb, Cr)*/

	/* zmienne zawierja tablice 3 elemntowa dla 3 skladowych obrazka (Y, Cb, Cr)
		kazdej skladowej przypozadkowana jest tablica blokow
		a w kazdy blok jest tablica 64 elementowa	*/
	int16_t *dev_res;
	int16_t *dc_res;

	cudaEvent_t start, stop; // pomiar czasu wykonania j�dra

	int16_t *host_tmp = (int16_t *)malloc(3 * width * height * sizeof(int16_t));
	size_t number_of_pixels = width * height;
	float elapsedTime = 0.0f;

	int grid_size_x = (int)(width / 8); /* liczba blokow watkow w sieci w kierunku x */
	int grid_size_y = (int)(height / 8); /* liczba blokow watkow w sieci w kierunku y */

	/* rozmiar siatki grid_size_x * grid_size_y * 3 skladowe obrazka (Y, Cb, Cr) */
	dim3 dimGrid(grid_size_x, grid_size_y, 3);
	/* rozmiar bloku watkow, zawieral bedzie 1 blok do obliczenia DCT*/
	dim3 dimBlock(8, 8);
	/* rozmiar pamieci wspoldzielonej przez 1 blok watkow */
	size_t shShize = (64 * sizeof(int8_t));

	checkCudaErrors(cudaSetDevice(0));

	/* Alokacja pamieci */

	/* alokacja danych wejsciowych do urzadzenia */
	checkCudaErrors(cudaMalloc((void **)&dev_image, 3 * (width * height * sizeof(int8_t))));

	/* alokacja dla danych wyjsciowych z urzadzenia*/
	checkCudaErrors(cudaMalloc((void **)&dev_res, 3 * (width * height * sizeof(int16_t))));

    checkCudaErrors(cudaMemcpyToSymbol(fdtbl_Y, (void *)dtY, (N) * sizeof(float)));
    checkCudaErrors(cudaMemcpyToSymbol(fdtbl_Cb, (void *)dtCb, (N) * sizeof(float)));

	/* kopiowanie pami�ci do urz�dzenia */
	checkCudaErrors(cudaMemcpy(dev_image + (width * height * Y), 
		Y_in, width * height * sizeof(int8_t), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(dev_image + (width * height * Cb), 
		Cb_in, width * height * sizeof(int8_t), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(dev_image + (width * height * Cr), 
		Cr_in, width * height * sizeof(int8_t), cudaMemcpyHostToDevice));

	checkCudaErrors(cudaEventCreate(&start));
	checkCudaErrors(cudaEventCreate(&stop));
	checkCudaErrors(cudaEventRecord(start, 0));


	/* wywolanie funkcji jadra */
	dtf_kernel << < dimGrid, dimBlock, shShize >> > (dev_res, dev_image, 
														width, height, number_of_pixels);

	checkCudaErrors(cudaGetLastError());

	checkCudaErrors(cudaEventRecord(stop, 0));
	checkCudaErrors(cudaEventSynchronize(stop));
	checkCudaErrors(cudaDeviceSynchronize());


	checkCudaErrors(cudaEventElapsedTime(&elapsedTime, start, stop));

	printf("GPU (kernel) time = %.3f ms \n",
		elapsedTime);


	/* Kopiowanie wynikow z pamieci urzadzenia do hosta */
	checkCudaErrors(cudaMemcpy(host_tmp, dev_res , 3 * width * height * sizeof(int16_t), cudaMemcpyDeviceToHost));


	/* zwolnienie pamieci */
	checkCudaErrors(cudaEventDestroy(start));
	checkCudaErrors(cudaEventDestroy(stop));
	cudaFree(dev_image);
	cudaFree(dev_res);

	checkCudaErrors(cudaDeviceReset());

	return host_tmp;
}
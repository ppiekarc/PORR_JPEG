#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ycc_conversion_with_dct.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "helper_cuda.h"
#include "helper_timer.h"

#define N 64
#define R_CHANNEL 0
#define G_CHANNEL 1
#define B_CHANNEL 2

#define Y_CHANNEL 0
#define Cb_CHANNEL 1
#define Cr_CHANNEL 2

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

__constant__ static float fdtbl_Y[N];
__constant__ static float fdtbl_Cb[N];

__constant__ static int32_t YR[256];
__constant__ static int32_t YG[256];
__constant__ static int32_t YB[256];

__constant__ static int32_t CbR[256];
__constant__ static int32_t CbG[256];
__constant__ static int32_t CbB[256];

__constant__ static int32_t CrR[256];
__constant__ static int32_t CrG[256];
__constant__ static int32_t CrB[256];

#define  Y(R,G,B) ((uint8_t)((YR[(R)] + YG[(G)] + YB[(B)]) >> 16 ) - 128)
#define Cb(R,G,B) ((uint8_t)((CbR[(R)] + CbG[(G)] + CbB[(B)]) >> 16 ))
#define Cr(R,G,B) ((uint8_t)((CrR[(R)] + CrG[(G)] + CrB[(B)]) >> 16 ))

#define image_(t, index) image[(t * width * height) + index]
#define result_(t, b, p) result[(t * width * height) + (b * 64) + p]

__global__ static void dtf_kernel(int16_t *result, uint8_t *image, size_t width, size_t height) {

    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp10, tmp11, tmp12, tmp13;
    float z1, z2, z3, z4, z5, z11, z13;
    float *dataptr;
    __shared__ float datafloat[64];
    float temp;
    float *fdtbl;
    int8_t ctr;
    size_t j = threadIdx.x;
    size_t i = threadIdx.y;

    const unsigned int channel_type = blockIdx.z;
    if (i == 0) {
        for (size_t i = 0; i < 8; i++) {
            size_t index = (blockIdx.x * 8) + i + (width * blockIdx.y * 8) + width * j;
            if (channel_type == Y_CHANNEL) {
                datafloat[i + (8 * j)] = (int8_t)Y(image_(R_CHANNEL, index), image_(G_CHANNEL, index), image_(B_CHANNEL, index));
            } else if (channel_type == Cb_CHANNEL) {
                datafloat[i + (8 * j)] = (int8_t)Cb(image_(R_CHANNEL, index), image_(G_CHANNEL, index), image_(B_CHANNEL, index));
            } else {
                datafloat[i + (8 * j)] = (int8_t)Cr(image_(R_CHANNEL, index), image_(G_CHANNEL, index), image_(B_CHANNEL, index));
            }
        }

        /* Pass 1: process rows. */
        dataptr = datafloat + (j * 8);
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
    }
    /* Pass 2: process columns */
    if (i == 1) {
        dataptr = datafloat + j;

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

        fdtbl = (channel_type == 0) ? fdtbl_Y : fdtbl_Cb;

        for (size_t i = 0; i < 8; i++) {
            /* quantization and scaling factor */
            temp = datafloat[i + (8 * j)] * fdtbl[i + (8 * j)];
            /* Round to nearest integer. */
            size_t block_nr = blockIdx.x + (gridDim.x * blockIdx.y);
            size_t pixel_nr = zigzag[i + 8 * j];
            result_(channel_type, block_nr, pixel_nr) = (int16_t)((int16_t)(temp + 16384.5) - 16384);
        }
    }
}

int16_t *ycc_conversion_with_dct(uint8_t *R, uint8_t *G, uint8_t *B, size_t width, size_t height, int *num_blocks,
                                 const float *dtY, const float *dtCb, int32_t *tYR, int32_t *tYG,
                                 int32_t *tYB, int32_t *tCbR, int32_t *tCbG, int32_t *tCbB, int32_t *tCrR,
                                 int32_t *tCrG, int32_t *tCrB) {

    uint8_t *dev_image; /* zawiera 3 skladowe obrazka (Y, Cb, Cr)*/

    /* zmienne zawierja tablice 3 elemntowa dla 3 skladowych obrazka (Y, Cb, Cr)
    kazdej skladowej przypozadkowana jest tablica blokow
    a w kazdy blok jest tablica 64 elementowa	*/
    int16_t *dev_res;
    int16_t *host_result;

    cudaEvent_t start, stop; // pomiar czasu wykonania j�dra
    float elapsedTime = 0.0f;

    int grid_size_x = (int)(width / 8); /* liczba blokow watkow w sieci w kierunku x */
    int grid_size_y = (int)(height / 8); /* liczba blokow watkow w sieci w kierunku y */

    /* rozmiar siatki grid_size_x * grid_size_y * 3 skladowe obrazka (Y, Cb, Cr) */
    dim3 dimGrid(grid_size_x, grid_size_y, 3);
    /* rozmiar bloku watkow, zawieral bedzie 1 blok do obliczenia DCT*/
    dim3 dimBlock(8, 2);
    /* rozmiar pamieci wspoldzielonej przez 1 blok watkow */
    size_t shShize = (64 * sizeof(int8_t));

    checkCudaErrors(cudaSetDevice(0));

    /* alokacja danych wejsciowych do urzadzenia */
    checkCudaErrors(cudaMalloc((void **)&dev_image, 3 * (width * height * sizeof(int8_t))));

    /* alokacja dla danych wyjsciowych z urzadzenia*/
    checkCudaErrors(cudaMalloc((void **)&dev_res, 3 * (width * height * sizeof(int16_t))));

    /* alokacj dla danych wyjsciowych do hosta */
    host_result = (int16_t *)malloc(3 * width * height * sizeof(int16_t));

    /* przekopiowanie stalych tablic kwantyzacji do urzadzenia */
    checkCudaErrors(cudaMemcpyToSymbol(fdtbl_Y, (void *)dtY, (N) * sizeof(float)));
    checkCudaErrors(cudaMemcpyToSymbol(fdtbl_Cb, (void *)dtCb, (N) * sizeof(float)));

    checkCudaErrors(cudaMemcpyToSymbol(YR, (void *)tYR, (256) * sizeof(int32_t)));
    checkCudaErrors(cudaMemcpyToSymbol(YB, (void *)tYB, (256) * sizeof(int32_t)));
    checkCudaErrors(cudaMemcpyToSymbol(YG, (void *)tYG, (256) * sizeof(int32_t)));

    checkCudaErrors(cudaMemcpyToSymbol(CbR, (void *)tCbR, (256) * sizeof(int32_t)));
    checkCudaErrors(cudaMemcpyToSymbol(CbG, (void *)tCbG, (256) * sizeof(int32_t)));
    checkCudaErrors(cudaMemcpyToSymbol(CbB, (void *)tCbB, (256) * sizeof(int32_t)));

    checkCudaErrors(cudaMemcpyToSymbol(CrR, (void *)tCrR, (256) * sizeof(int32_t)));
    checkCudaErrors(cudaMemcpyToSymbol(CrG, (void *)tCrG, (256) * sizeof(int32_t)));
    checkCudaErrors(cudaMemcpyToSymbol(CrB, (void *)tCrB, (256) * sizeof(int32_t)));

    /* kopiowanie pami�ci do urz�dzenia */
    checkCudaErrors(cudaMemcpy(dev_image + (width * height * R_CHANNEL), R, width * height * sizeof(int8_t), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(dev_image + (width * height * G_CHANNEL), G, width * height * sizeof(int8_t), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(dev_image + (width * height * B_CHANNEL), B, width * height * sizeof(int8_t), cudaMemcpyHostToDevice));

    checkCudaErrors(cudaEventCreate(&start));
    checkCudaErrors(cudaEventCreate(&stop));
    checkCudaErrors(cudaEventRecord(start, 0));

    dtf_kernel << < dimGrid, dimBlock, shShize >> > (dev_res, dev_image, width, height);

    checkCudaErrors(cudaGetLastError());

    checkCudaErrors(cudaEventRecord(stop, 0));
    checkCudaErrors(cudaEventSynchronize(stop));
    checkCudaErrors(cudaDeviceSynchronize());


    checkCudaErrors(cudaEventElapsedTime(&elapsedTime, start, stop));

//	printf("GPU (kernel) time = %.3f ms \n",
//		elapsedTime);

    /* Kopiowanie wynikow z pamieci urzadzenia do hosta */
    checkCudaErrors(cudaMemcpy(host_result, dev_res, 3 * width * height * sizeof(int16_t), cudaMemcpyDeviceToHost));

    checkCudaErrors(cudaEventDestroy(start));
    checkCudaErrors(cudaEventDestroy(stop));
    cudaFree(dev_image);
    cudaFree(dev_res);

    checkCudaErrors(cudaDeviceReset());

    return host_result;
}
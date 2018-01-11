#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ycc_converter.h"

int32_t YR[256], YG[256], YB[256];
int32_t CbR[256], CbG[256], CbB[256];
int32_t CrR[256], CrG[256], CrB[256];

#define  Y(R,G,B) ((uint8_t)((YR[(R)] + YG[(G)] + YB[(B)]) >> 16 ) - 128)
#define Cb(R,G,B) ((uint8_t)((CbR[(R)] + CbG[(G)] + CbB[(B)]) >> 16 ))
#define Cr(R,G,B) ((uint8_t)((CrR[(R)] + CrG[(G)] + CrB[(B)]) >> 16 ))


static bool ycbcrtables_precalculated = false;
void precalculate_YCbCr_tables()
{
    for (uint16_t R = 0; R <= 255; R++) {
        YR[R] = (int32_t)(65536 * 0.299 + 0.5) * R;
        CbR[R] = (int32_t)(65536 * -0.16874 + 0.5) * R;
        CrR[R] = (int32_t)(32768) * R;
    }
    for (uint16_t G = 0; G <= 255; G++) {
        YG[G] = (int32_t)(65536 * 0.587 + 0.5) * G;
        CbG[G] = (int32_t)(65536 * -0.33126 + 0.5) * G;
        CrG[G] = (int32_t)(65536 * -0.41869 + 0.5) * G;
    }
    for (uint16_t B = 0; B <= 255; B++) {
        YB[B] = (int32_t)(65536 * 0.114 + 0.5) * B;
        CbB[B] = (int32_t)(32768) * B;
        CrB[B] = (int32_t)(65536 * -0.08131 + 0.5) * B;
    }

    ycbcrtables_precalculated = true;
}

const ImageYCC const *convertImage(const ImageRGB *imageRGB) {

    if (ycbcrtables_precalculated == false) {
        precalculate_YCbCr_tables();
    }

    ImageYCC* const imageYCC = malloc(sizeof(ImageYCC));
    
    imageYCC->height = imageRGB->height;
    imageYCC->width = imageRGB->width;

    const size_t bitmap_size = imageRGB->width * imageRGB->height;
    imageYCC->Y = malloc(bitmap_size);
    imageYCC->Cb = malloc(bitmap_size);
    imageYCC->Cr = malloc(bitmap_size);
    
    for (size_t i = 0; i < bitmap_size; i++) {
        uint8_t R = imageRGB->R[i];
        uint8_t G = imageRGB->G[i];
        uint8_t B = imageRGB->B[i];

        imageYCC->Y[i] = Y(R, G, B);
        imageYCC->Cr[i] = Cr(R, G, B);
        imageYCC->Cb[i] = Cb(R, G, B);
    }
    
    return imageYCC;
}

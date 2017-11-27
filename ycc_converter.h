#pragma once

#include <stdint.h>
#include <stddef.h>

#include "bmp_loader.h"

typedef struct {
    int8_t *Y;
    int8_t *Cr;
    int8_t *Cb;
    size_t width;
    size_t height;
} ImageYCC;

const ImageYCC const *convert(const ImageRGB const *imageRGB);

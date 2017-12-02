#pragma once

#include <stdint.h>
#include <stddef.h>

#pragma pack(push,1)
typedef struct {
    int8_t magic[2];
    uint32_t filesize;
    int16_t reserved1;
    int16_t reserved2;
    uint32_t dataoffset;
    uint32_t headersize;
    uint32_t width;
    uint32_t height;
    int16_t planes;
    int16_t bitsperpixel;
    uint32_t compression;
    uint32_t bitmapsize;
    int32_t horizontalres;
    int32_t verticalres;
    uint32_t numcolors;
    uint32_t importantcolors;
} BmpFileHeader;
#pragma pack(pop)

typedef struct {
    uint8_t *R;
    uint8_t *G;
    uint8_t *B;
    size_t width;
    size_t height;
} ImageRGB;

const ImageRGB const *load_true_rgb_bitmap(const char *filename);
void release_bitmap(ImageRGB *imageRGB);

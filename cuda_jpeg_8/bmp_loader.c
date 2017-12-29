#include <stdio.h>
#include <stdlib.h>
#include "bmp_loader.h"

const ImageRGB const *load_true_rgb_bitmap(const char *const filename)
{
    FILE *f = fopen(filename, "rb");

    if (f == NULL) {
        fprintf(stderr, "Cannot open bitmap. File not found?\n");
        exit(EXIT_FAILURE);
    }

    BmpFileHeader bmp_file_header;
    fread(&bmp_file_header, sizeof(BmpFileHeader), 1, f);

    if (bmp_file_header.bitsperpixel != 24) {
        fprintf(stderr, "Need a truecolor BMP to compress!\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    if ((bmp_file_header.width % 8 != 0) || (bmp_file_header.height % 8 != 0)) {
        fprintf(stderr, "Only images sizes of 8 pixel multiple supported!\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    fseek(f, bmp_file_header.dataoffset, SEEK_SET);


    const size_t bitmap_size = bmp_file_header.width * bmp_file_header.height;
    uint8_t *const data = malloc(3 * bitmap_size);

    const uint32_t full_line_width = 3 * bmp_file_header.width;
    for (uint8_t *current_line = data + (3 * bitmap_size) - full_line_width;
         current_line >= data;
         current_line -= full_line_width) {
        fread(current_line, 1, full_line_width, f);
    }

    ImageRGB *image = malloc(sizeof(ImageRGB));
    image->width = bmp_file_header.width;
    image->height = bmp_file_header.height;
    image->R = malloc(bitmap_size);
    image->G = malloc(bitmap_size);
    image->B = malloc(bitmap_size);

    for (size_t i = 0; i < (3 * bitmap_size); i += 3) {
        image->R[i / 3] = data[i + 2];
        image->B[i / 3] = data[i];
        image->G[i / 3] = data[i + 1];
    }

    fclose(f);
    free(data);

    return image;
}

void release_bitmap(ImageRGB *imageRGB) {
    if (!imageRGB) {
        return;
    }

    free(imageRGB->R);
    free(imageRGB->G);
    free(imageRGB->B);

    free(imageRGB);
}

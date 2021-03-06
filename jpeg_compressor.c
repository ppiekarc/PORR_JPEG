#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "dct.h"
#include "timer.h"
#include "bmp_loader.h"
#include "ycc_converter.h"
#include "jpeg_writer.h"
#include "quantization.h"


static DHTinfo* construct_DHTInfo()
{
    DHTinfo *const dhTinfo =  malloc(sizeof(DHTinfo));
    dhTinfo->marker=0xFFC4;
    dhTinfo->length=0x01A2;
    dhTinfo->HTYDCinfo=0;
    for (size_t i = 0; i < 16; i++)  dhTinfo->YDC_nrcodes[i]=std_dc_luminance_nrcodes[i+1];
    for (size_t i = 0; i < 12; i++)  dhTinfo->YDC_values[i]=std_dc_luminance_values[i];
    dhTinfo->HTYACinfo=0x10;
    for (size_t i = 0; i < 16; i++)  dhTinfo->YAC_nrcodes[i]=std_ac_luminance_nrcodes[i+1];
    for (size_t i = 0; i < 162; i++) dhTinfo->YAC_values[i]=std_ac_luminance_values[i];
    dhTinfo->HTCbDCinfo=1;
    for (size_t i = 0; i < 16; i++) dhTinfo->CbDC_nrcodes[i]=std_dc_chrominance_nrcodes[i+1];
    for (size_t i = 0; i < 12; i++)  dhTinfo->CbDC_values[i]=std_dc_chrominance_values[i];
    dhTinfo->HTCbACinfo=0x11;
    for (size_t i = 0; i < 16; i++) dhTinfo->CbAC_nrcodes[i]=std_ac_chrominance_nrcodes[i+1];
    for (size_t i = 0; i < 162; i++) dhTinfo->CbAC_values[i]=std_ac_chrominance_values[i];

    return dhTinfo;
}

const static uint8_t QUANTIZATION_TABLE_SCALE_FACTOR = 50;
static channel_encoding_context yctx = {0};
static channel_encoding_context cbctx = {0};
static channel_encoding_context crctx = {0};

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <input_bitmap> <output_jpeg>\n", argv[0]);
        return 0;
    }

    const char *const bitmap_filename = argv[1];
    const ImageRGB *const rgb_image = load_true_rgb_bitmap(bitmap_filename);
    printf("Compressing %s using default JPEG parameters\n", bitmap_filename);

    app_timer_t start, stop;

    timer(&start);
    printf("[+] Converting %dx%d RGB bitmap into YCC color space\n", rgb_image->width, rgb_image->height);
    const ImageYCC *const image = convertImage(rgb_image);
    release_bitmap(rgb_image);

    const int number_of_dct_blocks = (int)roundf((image->width * image->height) / 64);
    printf("[+] Running DCT & Quantization on %i 8x8 blocks\n", number_of_dct_blocks);

    static uint8_t chrominance_quantization_table[64] = {};
    static uint8_t luminance_quantization_table[64] = {};
    scale_quantization_table_with_zigzag(std_luminance_qt, QUANTIZATION_TABLE_SCALE_FACTOR, luminance_quantization_table);
    scale_quantization_table_with_zigzag(std_chrominance_qt, QUANTIZATION_TABLE_SCALE_FACTOR, chrominance_quantization_table);

    const float *const fdtbl_Y = prepare_quantization_table(luminance_quantization_table);
    const float *const fdtbl_Cb = prepare_quantization_table(chrominance_quantization_table);

    int16_t **y_data_out = dct(image->Y, image->width, image->height, &number_of_dct_blocks, fdtbl_Y);
    int16_t **cb_data_out = dct(image->Cb, image->width, image->height, &number_of_dct_blocks, fdtbl_Cb);
    int16_t **cr_data_out = dct(image->Cr, image->width, image->height, &number_of_dct_blocks, fdtbl_Cb);

    printf("[+] Compressing using RLE with Huffman encoding\n");
    init_Huffman_tables();

    const size_t image_size = image->height * image->width;
    yctx.encoded_channel = malloc(image_size);
    cbctx.encoded_channel = malloc(image_size);
    crctx.encoded_channel = malloc(image_size);

    yctx.encoded_dct_indices = malloc(sizeof(size_t) * number_of_dct_blocks);
    cbctx.encoded_dct_indices = malloc(sizeof(size_t) * number_of_dct_blocks);
    crctx.encoded_dct_indices = malloc(sizeof(size_t) * number_of_dct_blocks);

    for (size_t block = 0; block < number_of_dct_blocks; block++) {
        encode_block(y_data_out[block], YDC_HT, YAC_HT, &yctx, block);
        encode_block(cb_data_out[block], CbDC_HT, CbAC_HT, &cbctx, block);
        encode_block(cr_data_out[block], CbDC_HT, CbAC_HT, &crctx, block);
    }

    const char *const jpeg_filename = argv[2];
    printf("[+] Writing JPEG to output file %s\n", jpeg_filename);

    static JpegFileDescriptor jpegFileDescriptor;
    jpegFileDescriptor.height = image->height;
    jpegFileDescriptor.width = image->width;
    jpegFileDescriptor.dqTable = malloc(sizeof(DQTable));
    jpegFileDescriptor.dqTable->marker = 0xFFDB;
    jpegFileDescriptor.dqTable->length = 132;
    jpegFileDescriptor.dqTable->QTYinfo = 0;
    jpegFileDescriptor.dqTable->QTCbinfo = 1;
    memcpy(jpegFileDescriptor.dqTable->Ytable, luminance_quantization_table, 64);
    memcpy(jpegFileDescriptor.dqTable->Cbtable, chrominance_quantization_table, 64);
    jpegFileDescriptor.dhTinfo = construct_DHTInfo();
    jpegFileDescriptor.cbctx = &cbctx;
    jpegFileDescriptor.yctx= &yctx;
    jpegFileDescriptor.crctx = &crctx;
    jpegFileDescriptor.bytepos = bytepos;
    jpegFileDescriptor.number_of_dct_blocks = number_of_dct_blocks;

    createJpegFile(jpeg_filename, &jpegFileDescriptor);

    timer(&stop);

    printf("Compression of %s completed in %.3f ms\n", bitmap_filename,  elapsed_time(start, stop));

#if defined(_WIN32) || defined(_WIN64)
    system("PAUSE");
#endif

    return 0;
}

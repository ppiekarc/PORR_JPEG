#include <stdio.h>
#include "jpeg_writer.h"

static uint8_t current_byte_position;
static int8_t byte_to_write = 0;

#define writebyte(b, filedesc) fputc((b), filedesc)
#define writeword(w, filedesc) \
    writebyte(((w)/256), filedesc); \
    writebyte(((w)%256), filedesc);

static uint16_t mask[16]={ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };

static void write_APP0Segment(FILE *fo)
{
    writeword(APP0Segment.marker, fo);
    writeword(APP0Segment.length, fo);
    writebyte('J', fo);
    writebyte('F', fo);
    writebyte('I', fo);
    writebyte('F', fo);
    writebyte(0, fo);
    writebyte(APP0Segment.versionhi, fo);
    writebyte(APP0Segment.versionlo, fo);
    writebyte(APP0Segment.xyunits, fo);
    writeword(APP0Segment.xdensity, fo);
    writeword(APP0Segment.ydensity, fo);
    writebyte(APP0Segment.thumbnwidth, fo);
    writebyte(APP0Segment.thumbnheight, fo);
}

static void write_DQTable(FILE *fo, const DQTable *const dqTable)
{
    writeword(dqTable->marker, fo);
    writeword(dqTable->length, fo);
    writebyte(dqTable->QTYinfo, fo);
    for (size_t i = 0; i < 64; i++) writebyte(dqTable->Ytable[i], fo);
    writebyte(dqTable->QTCbinfo, fo);
    for (size_t i = 0; i < 64; i++) writebyte(dqTable->Cbtable[i], fo);
}

static void write_SOF0info(FILE *fo, const uint16_t width, const uint16_t height)
{
    writeword(SOF0info.marker, fo);
    writeword(SOF0info.length, fo);
    writebyte(SOF0info.precision, fo);
    writeword(height, fo);
    writeword(width, fo);
    writebyte(SOF0info.nrofcomponents, fo);
    writebyte(SOF0info.IdY, fo);
    writebyte(SOF0info.HVY, fo);
    writebyte(SOF0info.QTY, fo);
    writebyte(SOF0info.IdCb, fo);
    writebyte(SOF0info.HVCb, fo);
    writebyte(SOF0info.QTCb, fo);
    writebyte(SOF0info.IdCr, fo);
    writebyte(SOF0info.HVCr, fo);
    writebyte(SOF0info.QTCr, fo);
}

static void write_DHTinfo(FILE *fo, const DHTinfo *const dhTinfo)
{
    size_t i;
    writeword(dhTinfo->marker, fo);
    writeword(dhTinfo->length, fo);
    writebyte(dhTinfo->HTYDCinfo, fo);
    for (i = 0; i < 16; i++) writebyte(dhTinfo->YDC_nrcodes[i], fo);
    for (i = 0; i < 12; i++) writebyte(dhTinfo->YDC_values[i], fo);
    writebyte(dhTinfo->HTYACinfo, fo);
    for (i = 0; i < 16; i++)  writebyte(dhTinfo->YAC_nrcodes[i], fo);
    for (i = 0; i < 162; i++) writebyte(dhTinfo->YAC_values[i], fo);
    writebyte(dhTinfo->HTCbDCinfo, fo);
    for (i = 0; i < 16; i++)  writebyte(dhTinfo->CbDC_nrcodes[i], fo);
    for (i = 0; i < 12; i++)  writebyte(dhTinfo->CbDC_values[i], fo);
    writebyte(dhTinfo->HTCbACinfo, fo);
    for (i = 0; i < 16; i++)  writebyte(dhTinfo->CbAC_nrcodes[i], fo);
    for (i = 0; i < 162; i++) writebyte(dhTinfo->CbAC_values[i], fo);
}

static void write_SOSinfo(FILE *fo)
{
    writeword(SOSinfo.marker, fo);
    writeword(SOSinfo.length, fo);
    writebyte(SOSinfo.nrofcomponents, fo);
    writebyte(SOSinfo.IdY, fo);
    writebyte(SOSinfo.HTY, fo);
    writebyte(SOSinfo.IdCb, fo);
    writebyte(SOSinfo.HTCb, fo);
    writebyte(SOSinfo.IdCr, fo);
    writebyte(SOSinfo.HTCr, fo);
    writebyte(SOSinfo.Ss, fo);
    writebyte(SOSinfo.Se, fo);
    writebyte(SOSinfo.Bf, fo);
}

static void writebits(const bitstring bs, FILE *fo) {
    int8_t current_bit_position = bs.length - 1;
    while (current_bit_position >= 0) {
        if (bs.value & mask[current_bit_position]) {
            byte_to_write |= mask[current_byte_position];
        }
        current_bit_position--;
        current_byte_position--;
        if (current_byte_position < 0) {
            if (byte_to_write == 0xFF) {
                writebyte(0xFF, fo);
                writebyte(0, fo);
            }
            else {
                writebyte(byte_to_write, fo);
            }
            current_byte_position = 7;
            byte_to_write = 0;
        }
    }
}

void createJpegFile(const char *const filename, const JpegFileDescriptor *const jpegFileDescriptor) {
    FILE *fo = fopen(filename, "wb");

    if (fo == NULL) {
        fprintf(stderr, "Cannot save JPEG to %s\n", filename);
        return;
    }

    writeword(SOI, fo);

    write_APP0Segment(fo);
    write_DQTable(fo, jpegFileDescriptor->dqTable);
    write_SOF0info(fo, (uint16_t)jpegFileDescriptor->width, (uint16_t)jpegFileDescriptor->height);
    write_DHTinfo(fo, jpegFileDescriptor->dhTinfo);
    write_SOSinfo(fo);

    const channel_encoding_context *const y_context = jpegFileDescriptor->yctx;
    const channel_encoding_context *const cb_context = jpegFileDescriptor->cbctx;
    const channel_encoding_context *const cr_context = jpegFileDescriptor->crctx;

    for (size_t i = 0; i < jpegFileDescriptor->number_of_dct_blocks - 1; i++) {
        fwrite(y_context->encoded_channel + y_context->encoded_dct_indices[i], 1, y_context->encoded_dct_indices[i + 1] - y_context->encoded_dct_indices[i], fo);
        fwrite(cb_context->encoded_channel + cb_context->encoded_dct_indices[i], 1, cb_context->encoded_dct_indices[i + 1] - cb_context->encoded_dct_indices[i], fo);
        fwrite(cr_context->encoded_channel + cr_context->encoded_dct_indices[i], 1, cr_context->encoded_dct_indices[i + 1] - cr_context->encoded_dct_indices[i], fo);
    }

    fwrite(y_context->encoded_channel + y_context->encoded_dct_indices[jpegFileDescriptor->number_of_dct_blocks - 1], 1,
           y_context->encoding_index - y_context->encoded_dct_indices[jpegFileDescriptor->number_of_dct_blocks - 1], fo);
    fwrite(cb_context->encoded_channel + cb_context->encoded_dct_indices[jpegFileDescriptor->number_of_dct_blocks - 1], 1,
           cb_context->encoding_index - cb_context->encoded_dct_indices[jpegFileDescriptor->number_of_dct_blocks - 1], fo);
    fwrite(cr_context->encoded_channel + cr_context->encoded_dct_indices[jpegFileDescriptor->number_of_dct_blocks - 1], 1,
           cr_context->encoding_index - cr_context->encoded_dct_indices[jpegFileDescriptor->number_of_dct_blocks - 1], fo);

    bitstring padding_bits;
    uint8_t byteposition = jpegFileDescriptor->bytepos;

    if (byteposition >= 0) {
        padding_bits.length = byteposition + 1;
        padding_bits.value = (1 << (byteposition + 1)) - 1;
        writebits(padding_bits, fo);
    }

    writeword(EOI, fo);

    fclose(fo);
}

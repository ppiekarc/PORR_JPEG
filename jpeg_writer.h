#pragma once

#include <stdint.h>
#include "huffman.h"

static const uint16_t SOI = 0xFFD8;
static const uint16_t EOI = 0xFFD9;

static struct APP0SegmentType {
    uint16_t marker;          /* APP0 marker (= 0xFFE0) */
    uint16_t length;          /* segment length (= 16 for usual JPEG) */
    uint8_t JFIFsignature[5]; /* identifier (= "JFIF",'\0') */
    uint8_t versionhi;        /* v1.1 version major */
    uint8_t versionlo;        /* v1.1 version minor */
    uint8_t xyunits;          /* density unit (= 0 - no units, normal density */
    uint16_t xdensity;        /* X density (= 1) */
    uint16_t ydensity;        /* Y density (= 1) */
    uint8_t thumbnwidth;      /* thumbnail width = 0 */
    uint8_t thumbnheight;     /* thumbnail height = 0 */
} APP0Segment = { 0xFFE0, 0x10, 'J', 'F', 'I', 'F', 0, 1, 1, 0, 1, 1, 0, 0 };

typedef struct {
    uint16_t marker;     /* DQT marker (= 0xFFDB) */
    uint16_t length;     /* segment length (= 0x43) */
    uint8_t QTYinfo;     /* bit 0..3: number of QT = 0, bit 4..7: precision of QT, 0 = 8 bit */
    uint8_t Ytable[64];  /* Quantization table for Y */
    uint8_t QTCbinfo;
    uint8_t Cbtable[64]; /* Quantization table for Cb,Cr */
} DQTable;

static struct {
    uint16_t marker; /* SOF0 marker (= 0xFFC0) */
    uint16_t length;
    uint8_t precision ; /* 8-bit precision */
    uint16_t height;
    uint16_t width;
    uint8_t nrofcomponents; /* = 3 (truecolor JPG) */
    uint8_t IdY;
    uint8_t HVY;
    uint8_t QTY;  /* Quantization Table number for Y (= 0) */
    uint8_t IdCb;
    uint8_t HVCb;
    uint8_t QTCb;
    uint8_t IdCr;
    uint8_t HVCr;
    uint8_t QTCr; /* Quantization Table number for QTCb (= 1) */
} SOF0info = { 0xFFC0, 17, 0x08, 0, 0, 3, 1, 0x11, 0, 2, 0x11, 1, 3, 0x11, 1 };

typedef struct {
    uint16_t marker;
    uint16_t length;
    uint8_t HTYDCinfo;
    uint8_t YDC_nrcodes[16];
    uint8_t YDC_values[12];
    uint8_t HTYACinfo;
    uint8_t YAC_nrcodes[16];
    uint8_t YAC_values[162];
    uint8_t HTCbDCinfo;
    uint8_t CbDC_nrcodes[16];
    uint8_t CbDC_values[12];
    uint8_t HTCbACinfo;
    uint8_t CbAC_nrcodes[16];
    uint8_t CbAC_values[162];
} DHTinfo;

static struct {
    uint16_t marker;
    uint16_t length;
    uint8_t nrofcomponents;
    uint8_t IdY;
    uint8_t HTY;
    uint8_t IdCb;
    uint8_t HTCb;
    uint8_t IdCr;
    uint8_t HTCr;
    uint8_t Ss,Se,Bf;
} SOSinfo={ 0xFFDA, 12, 3, 1, 0, 2, 0x11, 3, 0x11, 0, 0x3F ,0 };

typedef struct {
    DQTable *dqTable;
    DHTinfo *dhTinfo;
    size_t height;
    size_t width;
    channel_encoding_context *yctx;
    channel_encoding_context *cbctx;
    channel_encoding_context *crctx;
    int8_t bytepos;
    size_t number_of_dct_blocks;
} JpegFileDescriptor;

void createJpegFile(const char *filename, const JpegFileDescriptor *jpegFileDescriptor);

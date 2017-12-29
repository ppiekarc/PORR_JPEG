#include "huffman.h"

static uint16_t mask[16]={ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };

static size_t encoding_data_index = 0;
static uint8_t byte_to_write = 0;

/* category of the numbers in range: -32767..32767 */
static uint8_t categories_buffer[65535];
static uint8_t *const category = categories_buffer + 32767;
/* bitcoded representation of the numbers in range: -32767..32767 */
static bitstring bitcodes_buffer[65535];
static bitstring *const bitcode = bitcodes_buffer + 32767;

static void set_numbers_category_and_bitcode() {
    int32_t nrlower = 1,nrupper = 2;
    for (uint8_t c = 1; c <= 15; c++) {
        /* Positive numbers */
        for (int32_t nr = nrlower; nr < nrupper; nr++) {
            category[nr] = c;
            bitcode[nr].length = c;
            bitcode[nr].value = (uint16_t)nr;
        }
        /* Negative numbers */
        for (int32_t nr =- (nrupper-1); nr <= -nrlower; nr++) {
            category[nr] = c;
            bitcode[nr].length = c;
            bitcode[nr].value= (uint16_t)(nrupper - 1 + nr);
        }
        nrlower <<= 1;
        nrupper <<= 1;
    }
}

static void compute_Huffman_table(const uint8_t *const nrcodes, const uint8_t *const std_table, bitstring *const HT)
{
    size_t pos_in_table = 0;
    uint16_t codevalue = 0;
    for (size_t k = 1; k <= 16; k++) {
        for (size_t j = 1; j <= nrcodes[k]; j++) {
            HT[std_table[pos_in_table]].value = codevalue;
            HT[std_table[pos_in_table]].length = k;
            pos_in_table++;
            codevalue++;
        }
        codevalue *= 2;
    }
}

void init_Huffman_tables()
{
    set_numbers_category_and_bitcode();
    compute_Huffman_table(std_dc_luminance_nrcodes, std_dc_luminance_values, YDC_HT);
    compute_Huffman_table(std_dc_chrominance_nrcodes, std_dc_chrominance_values, CbDC_HT);
    compute_Huffman_table(std_ac_luminance_nrcodes, std_ac_luminance_values, YAC_HT);
    compute_Huffman_table(std_ac_chrominance_nrcodes, std_ac_chrominance_values, CbAC_HT);
}

static void writebyte(const uint8_t byte, uint8_t *const data) {
    data[encoding_data_index++] = byte;
}

static void writebits(const bitstring bs, uint8_t *const data) {
    int8_t current_bit_position = bs.length - 1;
    while (current_bit_position >= 0) {
        if (bs.value & mask[current_bit_position]) {
            byte_to_write |= mask[bytepos];
        }
        current_bit_position--;
        bytepos--;
        if (bytepos < 0) {
            if (byte_to_write == 0xFF) {
                writebyte(0xFF, data);
                writebyte(0, data);
            }
            else {
                writebyte(byte_to_write, data);
            }
            bytepos = 7;
            byte_to_write = 0;
        }
    }
}

void encode_block(const int16_t *const data, const bitstring *const HTDC, const bitstring *const HTAC, channel_encoding_context *const ctx, const size_t block) {
    const bitstring EOB = HTAC[0x00];
    const bitstring M16zeroes = HTAC[0xF0];
    encoding_data_index = ctx->encoding_index;
    ctx->encoded_dct_indices[block] = encoding_data_index;

    int16_t diff = data[0] - ctx->current_dc;
    ctx->current_dc  = data[0];
    if (diff == 0) {
        writebits(HTDC[0], ctx->encoded_channel);
    } else {
        writebits(HTDC[category[diff]], ctx->encoded_channel);
        writebits(bitcode[diff], ctx->encoded_channel);
    }

    size_t end0pos = 63;
    for (; (end0pos > 0) && (data[end0pos] == 0); end0pos--);
    if (end0pos==0) {
        writebits(EOB, ctx->encoded_channel);
        ctx->encoding_index = encoding_data_index;
        return;
    }

    size_t i = 1;
    while (i <= end0pos) {
        size_t startpos = i;
        for (; (data[i] == 0) && (i <= end0pos); i++);
        size_t nrzeroes = i - startpos;
        if (nrzeroes >= 16) {
            for (size_t nrmarker = 1; nrmarker <= nrzeroes / 16; nrmarker++) {
                writebits(M16zeroes, ctx->encoded_channel);
            }
            nrzeroes = nrzeroes % 16;
        }
        writebits(HTAC[nrzeroes * 16 + category[data[i]]], ctx->encoded_channel);
        writebits(bitcode[data[i]], ctx->encoded_channel);
        i++;
    }

    if (end0pos != 63) {
        writebits(EOB, ctx->encoded_channel);
    }

    ctx->encoding_index = encoding_data_index;
}

#pragma once

void dct_for_one_block(unsigned char *data_in, short *data_out);
short **dct_for_blocks(unsigned char *data_in, int w, int h, int *num_blocks);
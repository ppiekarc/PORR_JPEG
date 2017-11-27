cmake_minimum_required(VERSION 3.5)
project(PORR_JPEG)

add_executable(jpeg_seq jpeg.c dct.c jpeg_writer.c bmp_loader.h bmp_loader.c jpeg_rgb.c ycc_converter.c ycc_converter.h timer.c timer.h huffman.c huffman.h quantization.c quantization.h)
target_link_libraries(jpeg_seq m)

cmake_minimum_required(VERSION 3.5)
project(PORR_JPEG)

add_executable(jpeg_sequential_benchmark dct.c bmp_loader.c ycc_converter.c timer.c huffman.c quantization.c sequential_jpeg_benchmark.c)
target_link_libraries(jpeg_sequential_benchmark m)

add_executable(jpeg_parallel_benchmark dct.c bmp_loader.c ycc_converter.c timer.c huffman.c quantization.c parallel_jpeg_benchmark.c)
target_link_libraries(jpeg_parallel_benchmark m pthread)

add_executable(jpeg_compressor dct.c jpeg_writer.c bmp_loader.c ycc_converter.c timer.c huffman.c quantization.c jpeg_compressor.c)
target_link_libraries(jpeg_compressor m)

add_executable(jpeg_vectorized_benchmark dct.c bmp_loader.c ycc_converter.c timer.c huffman.c quantization.c vectorized_jpeg_benchmark.c)
target_link_libraries(jpeg_vectorized_benchmark m)
target_compile_options(jpeg_vectorized_benchmark PRIVATE -O3 -ftree-vectorize -mavx)

add_executable(jpeg_parallel_and_vectorized_benchmark dct.c bmp_loader.c ycc_converter.c timer.c huffman.c quantization.c parallel_and_vectorized_jpeg_benchmark.c)
target_link_libraries(jpeg_parallel_and_vectorized_benchmark m pthread)
target_compile_options(jpeg_parallel_and_vectorized_benchmark PRIVATE -O3 -ftree-vectorize -mavx)
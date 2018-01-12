project(PORR_JPEG)
cmake_minimum_required(VERSION 3.5)
find_package(CUDA QUIET REQUIRED)

SET(CUDA_PROPAGATE_HOST_FLAGS OFF)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -O3 -arch=sm_52 -gencode=arch=compute_52,code=sm_52)

string(REPLACE '--machine 32' '-m64' CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS}")

include_directories(kernels)
include_directories(/usr/local/cuda/samples/common/inc)

cuda_add_executable(cuda_jpeg_compressor
        cuda_jpeg_8/dct.c
        cuda_jpeg_8/bmp_loader.c
        cuda_jpeg_8/ycc_converter.c
        cuda_jpeg_8/timer.c
        cuda_jpeg_8/huffman.c
        cuda_jpeg_8/quantization.c
        cuda_jpeg_8/jpeg_compressor_CUDA.c
        cuda_jpeg_8/kernels/dctCUDAv2.cu
        cuda_jpeg_8/kernels/dctCUDAv2.h
        cuda_jpeg_8/jpeg_writer.c
        )

target_link_libraries(cuda_jpeg_compressor m)

cuda_add_executable(cuda_full_jpeg_compressor
        cuda_jpeg_8/dct.c
        cuda_jpeg_8/bmp_loader.c
        cuda_jpeg_8/ycc_converter.c
        cuda_jpeg_8/timer.c
        cuda_jpeg_8/huffman.c
        cuda_jpeg_8/quantization.c
        cuda_jpeg_8/jpeg_full_compressor_CUDA.c
        cuda_jpeg_8/kernels/ycc_conversion_with_dct.cu
        cuda_jpeg_8/kernels/ycc_conversion_with_dct.h
        cuda_jpeg_8/jpeg_writer.c
        )

target_link_libraries(cuda_full_jpeg_compressor m)

cuda_add_executable(cuda_jpeg_benchmark
        cuda_jpeg_8/dct.c
        cuda_jpeg_8/bmp_loader.c
        cuda_jpeg_8/ycc_converter.c
        cuda_jpeg_8/timer.c
        cuda_jpeg_8/huffman.c
        cuda_jpeg_8/quantization.c
        cuda_jpeg_8/cuda_jpeg_benchmark.c
        cuda_jpeg_8/kernels/dctCUDAv2.cu
        cuda_jpeg_8/kernels/dctCUDAv2.h
        )

target_link_libraries(cuda_jpeg_benchmark m)

cuda_add_executable(cuda_full_jpeg_benchmark
        cuda_jpeg_8/dct.c
        cuda_jpeg_8/bmp_loader.c
        cuda_jpeg_8/ycc_converter.c
        cuda_jpeg_8/timer.c
        cuda_jpeg_8/huffman.c
        cuda_jpeg_8/quantization.c
        cuda_jpeg_8/cuda_full_jpeg_benchmark.c
        cuda_jpeg_8/kernels/ycc_conversion_with_dct.cu
        cuda_jpeg_8/kernels/ycc_conversion_with_dct.h
        )

target_link_libraries(cuda_full_jpeg_benchmark m)
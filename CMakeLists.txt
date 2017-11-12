cmake_minimum_required(VERSION 3.5)
project(PORR_JPEG)

add_executable(jpeg_seq jpeg.c dct.c)
target_link_libraries(jpeg_seq m)

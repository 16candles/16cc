# CMakeLists.txt --- CMake file for 16cc
# Copyright (c) Joe Jevnik

cmake_minimum_required(VERSION 2.8)
project(16CANDLES)
set(16CANDLES_VERSION_MAJOR 0)
set(16CANDLES_VERSION_MINOR 1)

set(CMAKE_C_FLAGS
"-g -std=gnu11 -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -lpthread")

add_subdirectory(src)

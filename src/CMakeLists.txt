# CMakeLists.txt --- CMake file for 16cc
# Copyright (c) Joe Jevnik 2014

set(16CC_FILES compiler.c
               lexer.c)

set(EXECUTABLE_OUTPUT_PATH ${16CANDLES_BINARY_DIR})

add_executable(16cc ${16CC_FILES})

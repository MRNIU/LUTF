
# This file is a part of MRNIU/LUTF (https://github.com/MRNIU/LUTF).
#
# CMakeLists.txt for MRNIU/LUTF.

# Set minimum cmake version
cmake_minimum_required(VERSION 3.12)

project(LUTF C)

# Set C gnu11
set(CMAKE_C_STANDARD 11)

# Set common flags
# set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -g -ggdb -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -g -ggdb -O0")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Werror -O0")

set(LIBNAME lutf)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(${LUTF_SOURCE_DIR} STREQUAL ${LUTF_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()
message(STATUS "C compiler: ${CMAKE_C_COMPILER_ID}, version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C FLAGS: ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "lib dir: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message(STATUS "test dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

set(LUTF_SOURCE_CODE_DIR ${LUTF_SOURCE_DIR}/src)

add_subdirectory(${LUTF_SOURCE_CODE_DIR})
add_subdirectory(${LUTF_SOURCE_DIR}/test)


# This file is a part of MRNIU/LUTF (https://github.com/MRNIU/LUTF).
#
# CMakeLists.txt for MRNIU/LUTF.

# Set minimum cmake version
cmake_minimum_required(VERSION 3.12)

project(LUTF C)

if(${LUTF_SOURCE_DIR} STREQUAL ${LUTF_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# Set debug
set(CMAKE_BUILD_TYPE DEBUG)

# Set C gnu11
set(CMAKE_C_STANDARD 11)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_C_COMPILER gcc)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CMAKE_C_COMPILER gcc-10)
endif ()

# Set common flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(LIBNAME lutf)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(LUTF_SOURCE_CODE_DIR ${LUTF_SOURCE_DIR}/src)

add_subdirectory(${LUTF_SOURCE_CODE_DIR})
add_subdirectory(${LUTF_SOURCE_DIR}/test)

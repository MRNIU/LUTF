
# This file is a part of MRNIU/LUTF (https://github.com/MRNIU/LUTF).
#
# CMakeLists.txt for MRNIU/LUTF.

if (CMAKE_BUILD_TYPE STREQUAL DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb -O0 -fprofile-arcs -ftest-coverage")
endif ()

message(STATUS "CMAKE_C_FLAGS for lib is ${CMAKE_C_FLAGS}")

include_directories(${LUTF_SOURCE_DIR}/include)

# aux_source_directory(${LUTF_SOURCE_CODE_DIR} src)
set(src
    ${LUTF_SOURCE_CODE_DIR}/lutf.c)

add_library(${LIBNAME} ${src})

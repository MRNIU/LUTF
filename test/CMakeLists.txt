
# This file is a part of MRNIU/LUTF (https://github.com/MRNIU/LUTF).
#
# CMakeLists.txt for MRNIU/LUTF.

if (CMAKE_BUILD_TYPE STREQUAL DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb -O0 -lpthread -fprofile-arcs -ftest-coverage")
endif ()

message(STATUS "CMAKE_C_FLAGS for test is ${CMAKE_C_FLAGS}")

include_directories(${LUTF_SOURCE_DIR}/include)

aux_source_directory(${LUTF_SOURCE_DIR}/test src)

set(test_lutf_src
    test_lutf.c)

set(test_normal_src
    test_normal.c)

link_libraries(${LIBNAME})

add_executable(test_lutf ${test_lutf_src})

add_executable(test_normal ${test_normal_src})

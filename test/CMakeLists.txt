
# This file is a part of MRNIU/LUTF (https://github.com/MRNIU/LUTF).
#
# CMakeLists.txt for MRNIU/LUTF.

include_directories(${LUTF_SOURCE_DIR}/include)

aux_source_directory(${LUTF_SOURCE_DIR}/test src)

set(test_src
    test.c
    fifo.c
    time.c)

link_libraries(${LIBNAME})

add_executable(test ${test_src})


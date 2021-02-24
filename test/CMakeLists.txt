
# This file is a part of MRNIU/LUTF (https://github.com/MRNIU/LUTF).
#
# CMakeLists.txt for MRNIU/LUTF.

include_directories(${LUTF_SOURCE_DIR}/include)

aux_source_directory(${LUTF_SOURCE_DIR}/test src)

set(test_src
    test.c
    test_task.c
    test_manage.c
    test_sched.c
    test_io.c
    test_fifo.c)

link_libraries(${LIBNAME})

add_executable(test ${test_src})



# This file is a part of MRNIU/LUTF (https://github.com/MRNIU/LUTF).
#
# CMakeLists.txt for MRNIU/LUTF.

include_directories(${LUTF_SOURCE_DIR}/include)

set(src
    ${LUTF_SOURCE_CODE_DIR}/lutf.c)

add_library(${LIBNAME} SHARED ${src})

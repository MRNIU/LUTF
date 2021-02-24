
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// test_io.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "lutf.h"

static int test1(void *argv __attribute__((unused))) {
    return 0;
}

static int basic(void) {
    return 0;
}

int test_io(void) {
    printf("--------io--------\n");
    assert(basic() == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif

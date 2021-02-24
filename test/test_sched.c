
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// test_sched.c for MRNIU/LUTF.

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

int test_sched(void) {
    printf("--------sched--------\n");
    assert(basic() == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif

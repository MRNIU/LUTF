
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// test.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "assert.h"
#include "stdio.h"
#include "test.h"

static int run_test(UnitTestFunction test) {
    return test();
}

void run_tests(UnitTestFunction *tests) {
    for (int i = 0; tests[i] != NULL; i++) {
        assert(run_test(tests[i]) == 0);
    }
    return;
}

static UnitTestFunction tests[] = {
    test_task, test_manage, test_sched, test_io, test_fifo, NULL,
};

int main(int    argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
    run_tests(tests);
    printf("test done\n");
    return 0;
}

#ifdef __cplusplus
}
#endif


// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// lutf.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "lutf.h"

static uint32_t i = 0;

void *fun1(void *argv) {
    unsigned long long a = ((unsigned long long *)argv)[0];
    printf("%llu\n", a);
    if (a == 2000000) {
        ;
    }
    return NULL;
}

void *fun2(void *argv) {
    uint64_t j = 0;
    j          = i * i * i / 2 + 34;
    i++;
    if (i % 1000 == 0) {
        printf("i: %d\n", i);
    }
    return NULL;
}

void *fun3(void *argv) {
    int a = ((int *)argv)[0];
    printf("%d\n", a);
    if (a % 1000 == 0) {
        printf("a: %d\n", a);
    }
    return NULL;
}

#define COUNT 500000

int main(int argc __unused, char **argv __unused) {
    RETCODE_t ret = SUCCESS;
    ret           = lutf_init();
    if (ret != SUCCESS) {
        printf("init error\n");
    }
    lutf_thread_t *t[COUNT];
    int            ar[COUNT];
    for (int k = 0; k < COUNT; k++) {
        ar[k] = k;
        t[k]  = lutf_create_task(fun3, &ar[k]);
    }
    for (int k = COUNT - 1; k >= 0; k--) {
        ret = lutf_run(t[k]);
        if (ret != SUCCESS) {
            printf("Add sched error\n");
        }
    }
    printf("End.");
    return 0;
}

#ifdef __cplusplus
}
#endif

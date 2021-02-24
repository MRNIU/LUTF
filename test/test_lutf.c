
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// lutf.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
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

void *fun2(void *argv __attribute__((unused))) {
    i++;
    if (i % 1000 == 0) {
        printf("i: %d\n", i);
    }
    return NULL;
}

void *fun3(void *argv) {
    uint32_t a            = ((uint32_t *)argv)[0];
    ((uint32_t *)argv)[0] = a * a;
    // printf("%d\n", a);
    if (a % 1000 == 0) {
        printf("a: %d\n", a);
    }
    return NULL;
}

#define COUNT 800000

int main(int    argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
    int init = lutf_init();
    if (init != 0) {
        printf("init error\n");
    }
    lutf_thread_t *thread =
        (lutf_thread_t *)malloc(COUNT * sizeof(lutf_thread_t));

    uint32_t ar[COUNT];
    for (uint32_t k = 0; k < COUNT; k++) {
        ar[k] = k;
        lutf_create_task(&thread[k], fun3, &ar[k]);
    }
    for (uint32_t k = 0; k < COUNT; k++) {
        if (lutf_run(&thread[k]) != SUCCESS) {
            printf("Add sched error\n");
        }
    }
    for (uint32_t k = 0; k < COUNT; k++) {
        assert(ar[k] == k * k);
    }
    free(thread);
    printf("End.\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

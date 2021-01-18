
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// lutf.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "unistd.h"
#include "assert.h"
#include "pthread.h"
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

void *fun2(void *argv UNUSED) {
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

int main(int argc UNUSED, char **argv UNUSED) {
    pthread_t *threads = (pthread_t *)malloc(COUNT * sizeof(pthread_t));
    int        ar[COUNT];
    for (int k = 0; k < COUNT; k++) {
        ar[k] = k;
        pthread_create(&threads[k], NULL, fun3, &ar[k]);
    }
    for (int k = 0; k < COUNT; k++) {
        if (pthread_join(threads[k], NULL)) {
            printf("thread is not exit...\n");
            return 0;
        }
    }
    for (int k = 0; k < COUNT; k++) {
        assert(ar[k] == k * k);
    }
    free(threads);
    printf("End.\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

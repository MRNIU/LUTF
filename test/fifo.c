
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// test_basic.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "unistd.h"
#include "time.h"
#include "string.h"
#include "lutf.h"

#define C 6400000
static size_t test1_count = 0;
static size_t test2_count = 0;
static size_t test3_count = 0;

static void *test1(void *arg) {
    if (arg != NULL) {
        *(char *)arg;
    }
    test1_count++;
    lutf_exit((void *)"This is test1 exit value");
    assert(0);
    return NULL;
}

static void *test2(void *arg) {
    if (arg != NULL) {
        *(char *)arg;
    }
    // Do some calculations
    for (size_t i = 0; i < C; i++) {
        ;
    }
    test2_count++;
    lutf_exit((void *)"This is test2 exit value");
    assert(0);
    return NULL;
}

static void *test3(void *arg) {
    if (arg != NULL) {
        *(char *)arg;
    }
    test3_count++;
    lutf_exit((void *)"This is test3 exit value");
    assert(0);
    return NULL;
}

static int _join_exit(void) {
    lutf_t *threads = malloc(3 * sizeof(lutf_t));
    char *  arg[3]  = {"This is test1 arg", "This is test2 arg",
                    "This is test3 arg"};
    void ** ret     = malloc(3 * sizeof(char *));
    assert(lutf_create(&threads[0], test1, (void *)arg[0]) == 0);
    assert(lutf_create(&threads[1], test2, (void *)arg[1]) == 0);
    assert(lutf_create(&threads[2], test3, (void *)arg[2]) == 0);
    lutf_join(&threads[0], &ret[0]);
    assert(strcmp("This is test1 exit value", (char *)ret[0]) == 0);
    lutf_join(&threads[1], &ret[1]);
    assert(strcmp("This is test2 exit value", (char *)ret[1]) == 0);
    lutf_join(&threads[2], &ret[2]);
    assert(strcmp("This is test3 exit value", (char *)ret[2]) == 0);
    assert(test1_count == 1);
    assert(test2_count == 1);
    assert(test3_count == 1);
    return 0;
}

static int _equal(void) {
    lutf_t *threads = malloc(3 * sizeof(lutf_t));
    assert(lutf_create(&threads[0], test1, NULL) == 0);
    assert(lutf_create(&threads[1], test2, NULL) == 0);
    assert(lutf_create(&threads[2], test3, NULL) == 0);
    lutf_join(&threads[0], NULL);
    lutf_join(&threads[1], NULL);
    lutf_join(&threads[2], NULL);
    assert(lutf_equal(&threads[0], &threads[0]) == 1);
    assert(lutf_equal(&threads[0], &threads[1]) == 0);
    assert(lutf_equal(&threads[0], &threads[2]) == 0);
    assert(lutf_equal(&threads[1], &threads[0]) == 0);
    assert(lutf_equal(&threads[1], &threads[1]) == 1);
    assert(lutf_equal(&threads[1], &threads[2]) == 0);
    assert(lutf_equal(&threads[2], &threads[0]) == 0);
    assert(lutf_equal(&threads[2], &threads[1]) == 0);
    assert(lutf_equal(&threads[2], &threads[2]) == 1);
    return 0;
}

static size_t test4_count = 0;
static size_t test5_count = 0;
static size_t test6_count = 0;
static void * test4(void *arg __attribute__((unused))) {
    int index = 3;
    test4_count++;
    while (1) {
        if (--index == 0) {
            lutf_t r = lutf_self();
            lutf_cancel(&r);
        }
    }
    assert(0);
    return NULL;
}

static void *test5(void *arg __attribute__((unused))) {
    int index = 3;
    test5_count++;
    while (1) {
        if (--index == 0) {
            lutf_t r = lutf_self();
            lutf_cancel(&r);
        }
    }
    assert(0);
    return NULL;
}

static void *test6(void *arg __attribute__((unused))) {
    int index = 3;
    test6_count++;
    while (1) {
        if (--index == 0) {
            lutf_t r = lutf_self();
            lutf_cancel(&r);
        }
    }
    assert(0);
    return NULL;
}

static int _cancel_self(void) {
    lutf_t *threads = malloc(3 * sizeof(lutf_t));
    assert(lutf_create(&threads[0], test4, NULL) == 0);
    assert(lutf_create(&threads[1], test5, NULL) == 0);
    assert(lutf_create(&threads[2], test6, NULL) == 0);
    lutf_join(&threads[0], NULL);
    lutf_join(&threads[1], NULL);
    lutf_join(&threads[2], NULL);
    assert(test4_count == 1);
    assert(test5_count == 1);
    assert(test6_count == 1);
    return 0;
}

static size_t test7_count = 0;
static size_t test8_count = 0;
static void * test7(void *arg) {
    printf("test7\n");
    if (arg != NULL) {
        printf("arg: %d\n", *(uint32_t *)arg);
    }
    test7_count++;
    lutf_exit(arg);
    return NULL;
}

static void *test8(void *arg) {
    printf("test8\n");
    if (arg != NULL) {
        printf("arg: %d\n", *(uint32_t *)arg);
    }
    // Do some calculations
    for (size_t i = 0; i < C; i++) {
        ;
    }
    test8_count++;
    lutf_exit(arg);
    return NULL;
}

static int _million(void) {
#define COUNT 100
    lutf_t *  threads = malloc(COUNT * sizeof(lutf_t));
    void **   ret     = malloc(COUNT * sizeof(uint32_t *));
    uint32_t *arg     = (uint32_t *)malloc(COUNT * sizeof(uint32_t));
    for (size_t i = 0; i < COUNT; i++) {
        arg[i] = i;
        ret[i] = NULL;
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        assert(lutf_create(&threads[i], test7, (void *)&arg[i]) == 0);
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        lutf_join(&threads[i], &ret[i]);
        assert(*(uint32_t *)ret[i] == i);
    }
    for (int i = 0; i < C; i++) {
        ;
    }
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        assert(lutf_create(&threads[i], test8, (void *)&arg[i]) == 0);
    }
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        lutf_join(&threads[i], &ret[i]);
        assert(*(uint32_t *)ret[i] == i);
    }
    assert(test7_count == COUNT / 2);
    assert(test8_count == COUNT / 2);
    return 0;
}

int fifo(void) {
    printf("--------FIFO--------\n");
    printf("In this mode, threads are execute sequentially.\n");
    printf("----join_exit----\n");
    printf("Create a thread, run and output its return value.\n");
    printf("Functions used are: lutf_create, lutf_join, lutf_exit.\n");
    assert(_join_exit() == 0);
    printf("----equal----\n");
    printf("Determine whether two threads are the same.\n");
    assert(_equal() == 0);
    printf("----cancel_self----\n");
    printf("Cancel a thread itself\n");
    assert(_cancel_self() == 0);
    printf("----million----\n");
    printf("Create %d threads, run and output its return value.\n", COUNT);
    printf("Functions used are: lutf_create, lutf_join, lutf_exit.\n");
    assert(_million() == 0);
    printf("--------FIFO END--------\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

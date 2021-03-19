
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

static void *test1(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    for (size_t i = 0; i < 100000; i++) {
        printf("test1\n");
    }
    lutf_exit((void *)"This is test1 exit value");
    return NULL;
}

static void *test2(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    // Do some calculations
    for (size_t i = 0; i < 100000; i++) {
        printf("test2\n");
    }
    lutf_exit((void *)"This is test2 exit value");
    return NULL;
}

static void *test3(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    for (size_t i = 0; i < 100000; i++) {
        printf("test3\n");
    }
    lutf_exit((void *)"This is test3 exit value");
    return NULL;
}

static void *test4(void *arg) {
    printf("test4\n");
    if (arg != NULL) {
        printf("arg: %d\n", *(uint32_t *)arg);
    }
    lutf_exit(arg);
    return NULL;
}

static void *test5(void *arg) {
    printf("test5\n");
    if (arg != NULL) {
        printf("arg: %d\n", *(uint32_t *)arg);
    }
    // Do some calculations
    for (size_t i = 0; i < CLOCKS_PER_SEC; i++) {
        ;
    }
    lutf_exit(arg);
    return NULL;
}

static int _detach_exit_wait(void) {
    lutf_thread_t *threads = (lutf_thread_t *)malloc(3 * sizeof(lutf_thread_t));
    char *         arg[3]  = {"This is test1 arg", "This is test2 arg",
                    "This is test3 arg"};
    assert(lutf_create(&threads[0], test1, (void *)arg[0]) == 0);
    assert(lutf_create(&threads[1], test2, (void *)arg[1]) == 0);
    assert(lutf_create(&threads[2], test3, (void *)arg[2]) == 0);
    // 开始运行
    lutf_detach(&threads[0]);
    lutf_detach(&threads[1]);
    lutf_detach(&threads[2]);
    // 等待退出
    printf("wait begin\n");
    lutf_wait(threads, 3);
    printf("end\n");
    return 0;
}

// static int _self(void) {
//     return 0;
// }
// static int _equal(void) {
//     return 0;
// }
// static int _cancel(void) {
//     return 0;
// }
// static int _sync(void) {
//     return 0;
// }

// 百万级测试
static int _million(void) {
#define COUNT 2000
    lutf_thread_t *threads =
        (lutf_thread_t *)malloc(COUNT * sizeof(lutf_thread_t));
    uint32_t *arg = (uint32_t *)malloc(COUNT * sizeof(uint32_t));
    for (size_t i = 0; i < COUNT; i++) {
        arg[i] = i;
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        assert(lutf_create(&threads[i], test4, (void *)&arg[i]) == 0);
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        lutf_detach(&threads[i]);
    }
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        assert(lutf_create(&threads[i], test5, (void *)&arg[i]) == 0);
    }
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        lutf_detach(&threads[i]);
        lutf_wait(&threads[i], 1);
    }
    return 0;
}

// 测试顺序
// create，join，exit，wait，self，equal，cancel，sync
int time_(void) {
    printf("--------TIME--------\n");
    printf(
        "In this mode, threads will be scheduled based on time resources.\n");
    lutf_set_sched(TIME);
    printf("----detach_exit_wait----\n");
    printf("Create a thread, run and output its return value.\n");
    printf("Functions used: lutf_create, lutf_detach, lutf_wait, lutf_exit.\n");
    assert(_detach_exit_wait() == 0);
    // printf("----self----\n");
    // assert(_self() == 0);
    // printf("----equal----\n");
    // assert(_equal() == 0);
    // printf("----cancel----\n");
    // assert(_cancel() == 0);
    // printf("----sync----\n");
    // assert(_sync() == 0);
    // printf("----million----\n");
    // printf("Create a million threads, run and output its return value.\n");
    // printf("Functions used are: lutf_create, lutf_detach, lutf_exit.\n");
    // assert(_million() == 0);
    printf("--------TIME END--------\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

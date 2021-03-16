
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
    for (size_t i = 0; i < 10000; i++) {
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
    for (size_t i = 0; i < 10000; i++) {
        printf("test2\n");
    }
    lutf_exit((void *)"This is test2 exit value");
    return NULL;
}

static void *test3(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    for (size_t i = 0; i < 10000; i++) {
        printf("test3\n");
    }
    lutf_exit((void *)"This is test3 exit value");
    return NULL;
}

// static void *test4(void *arg) {
//     printf("test4\n");
//     if (arg != NULL) {
//         printf("arg: %d\n", *(uint32_t *)arg);
//     }
//     lutf_exit(arg);
//     return NULL;
// }

// static void *test5(void *arg) {
//     printf("test5\n");
//     if (arg != NULL) {
//         printf("arg: %d\n", *(uint32_t *)arg);
//     }
//     // Do some calculations
//     for (size_t i = 0; i < CLOCKS_PER_SEC; i++) {
//         ;
//     }
//     lutf_exit(arg);
//     return NULL;
// }

static int _detach_exit_wait(void) {
    lutf_thread_t *threads = (lutf_thread_t *)malloc(3 * sizeof(lutf_thread_t));
    char *         arg[3]  = {"This is test1 arg", "This is test2 arg",
                    "This is test3 arg"};
    void **        ret     = malloc(3 * sizeof(char *));
    assert(lutf_create(&threads[0], test1, (void *)arg[0]) == 0);
    assert(lutf_create(&threads[1], test2, (void *)arg[1]) == 0);
    assert(lutf_create(&threads[2], test3, (void *)arg[2]) == 0);
    // 开始运行
    lutf_detach(&threads[0], &ret[0]);
    lutf_detach(&threads[1], &ret[1]);
    lutf_detach(&threads[2], &ret[2]);
    // 等待退出
    lutf_wait(threads, 3);
    // assert(strcmp("This is test1 exit value", (char *)ret[0]) == 0);
    // assert(strcmp("This is test2 exit value", (char *)ret[1]) == 0);
    // assert(strcmp("This is test3 exit value", (char *)ret[2]) == 0);
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
// static int _million(void) {
// #define COUNT 4
//     lutf_thread_t *threads =
//         (lutf_thread_t *)malloc(COUNT * sizeof(lutf_thread_t));
//     void **   ret = malloc(COUNT * sizeof(uint32_t *));
//     uint32_t *arg = (uint32_t *)malloc(COUNT * sizeof(uint32_t));
//     for (size_t i = 0; i < COUNT; i++) {
//         arg[i] = i;
//         ret[i] = NULL;
//     }
//     for (size_t i = 0; i < COUNT / 2; i++) {
//         assert(lutf_create(&threads[i], test4, (void *)&arg[i]) == 0);
//     }
//     for (size_t i = 0; i < COUNT / 2; i++) {
//         lutf_detach(&threads[i], &ret[i]);
//         assert(*(uint32_t *)ret[i] == i);
//     }
//     for (size_t i = COUNT / 2; i < COUNT; i++) {
//         assert(lutf_create(&threads[i], test5, (void *)&arg[i]) == 0);
//     }
//     for (size_t i = COUNT / 2; i < COUNT; i++) {
//         lutf_detach(&threads[i], &ret[i]);
//         assert(*(uint32_t *)ret[i] == i);
//     }
//     return 0;
// }

// 测试顺序
// create，join，exit，wait，self，equal，cancel，sync
int time_(void) {
    printf("--------TIME--------\n");
    printf(
        "In this mode, threads will be scheduled based on time resources.\n");
    lutf_set_sched(TIME);
    printf("----join_exit----\n");
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

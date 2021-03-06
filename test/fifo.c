
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
    printf("test1\n");
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    lutf_exit((void *)"This is test1 exit value");
    return NULL;
}

static void *test2(void *arg) {
    printf("test2\n");
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    lutf_exit((void *)"This is test2 exit value");
    return NULL;
}

static void *test3(void *arg) {
    printf("test3\n");
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
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

static int _create(void) {
    lutf_thread_t task[3];
    assert(lutf_create(&task[0], test1, NULL) == 0);
    assert(lutf_create(&task[1], test2, NULL) == 0);
    assert(lutf_create(&task[2], test3, NULL) == 0);
    char *status[] = {
        "RUNNING", "EXIT", "WAIT", "SEM", "SLEEP",
    };
    for (int i = 0; i < 3; i++) {
        printf("id: %d, status: %s, func: %p, arg: %p, exit_value: %p, "
               "constext: "
               "%p, prev: %p, next: %p, waited: %p\n",
               task[i].id, status[task[i].status], task[i].func, task[i].arg,
               task[i].exit_value, task[i].context, task[i].prev, task[i].next,
               task[i].waited);
    }
    return 0;
}

static int _join_exit(void) {
    lutf_thread_t *threads = (lutf_thread_t *)malloc(3 * sizeof(lutf_thread_t));
    void **        ret     = malloc(3 * sizeof(uint32_t *));
    char *         arg[3]  = {"This is test1 arg", "This is test2 arg",
                    "This is test3 arg"};
    assert(lutf_create(&threads[0], test1, arg[0]) == 0);
    assert(lutf_create(&threads[1], test2, arg[1]) == 0);
    assert(lutf_create(&threads[2], test3, arg[2]) == 0);
    lutf_join(&threads[0], &ret[0]);
    assert(strcmp("This is test1 exit value", (char *)ret[0]) == 0);
    lutf_join(&threads[1], &ret[1]);
    assert(strcmp("This is test2 exit value", (char *)ret[1]) == 0);
    lutf_join(&threads[2], &ret[2]);
    assert(strcmp("This is test3 exit value", (char *)ret[2]) == 0);
    return 0;
}

// 百万级测试
static int _million(void) {
#define COUNT 16
    lutf_thread_t *threads =
        (lutf_thread_t *)malloc(COUNT * sizeof(lutf_thread_t));
    void **   ret = malloc(COUNT * sizeof(uint32_t *));
    uint32_t *arg = (uint32_t *)malloc(COUNT * sizeof(uint32_t));
    for (size_t i = 0; i < COUNT; i++) {
        arg[i] = i;
        ret[i] = NULL;
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        assert(lutf_create(&threads[i], test4, (void *)&arg[i]) == 0);
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        lutf_join(&threads[i], &ret[i]);
        assert(*(uint32_t *)ret[i] == i);
    }
    printf("Wait a second.\n");
    for (int i = 0; i < CLOCKS_PER_SEC; i++) {
        ;
    }
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        assert(lutf_create(&threads[i], test5, (void *)&arg[i]) == 0);
    }
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        lutf_join(&threads[i], &ret[i]);
        assert(*(uint32_t *)ret[i] == i);
    }
    return 0;
}

int fifo(void) {
    printf("--------FIFO--------\n");
    printf("In this mode, threads are execute sequentially.\n");
    lutf_set_sched(FIFO);
    printf("----create----\n");
    printf("Create a thread and output its internal data.\n");
    printf("Functions used are: lutf_create.\n");
    assert(_create() == 0);
    printf("----join_exit----\n");
    printf("Create a thread, run and output its return value.\n");
    printf("Functions used are: lutf_create, lutf_join, lutf_exit.\n");
    assert(_join_exit() == 0);
    printf("----million----\n");
    printf("Create a million threads, run and output its return value.\n");
    printf("Functions used are: lutf_create, lutf_join, lutf_exit.\n");
    assert(_million() == 0);
    printf("--------FIFO END--------\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

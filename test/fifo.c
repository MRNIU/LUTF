
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
#include "lutf.h"

static void *test1(void *arg) {
    printf("test1\n");
    printf("arg: %s\n", (char *)arg);
    lutf_exit((void *)"This is test1 exit value");
    return NULL;
}

static void *test2(void *arg) {
    printf("test2\n");
    printf("arg: %s\n", (char *)arg);
    lutf_exit((void *)"This is test2 exit value");
    return NULL;
}

static void *test3(void *arg) {
    printf("test3\n");
    printf("arg: %s\n", (char *)arg);
    lutf_exit((void *)"This is test3 exit value");
    return NULL;
}

static void *test4(void *arg) {
    printf("test4\n");
    printf("arg: %d\n", *(uint32_t *)arg);
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
    assert(lutf_init() == 0);
    lutf_thread_t task[3];
    void *        ret[3];
    char *        arg[3] = {"This is test1 arg", "This is test2 arg",
                    "This is test3 arg"};
    assert(lutf_create(&task[0], test1, arg[0]) == 0);
    assert(lutf_create(&task[1], test2, arg[1]) == 0);
    assert(lutf_create(&task[2], test3, arg[2]) == 0);
    lutf_join(&task[0], &ret[0]);
    printf("%s\n", (char *)ret[0]);
    lutf_join(&task[1], &ret[1]);
    printf("%s\n", (char *)ret[1]);
    lutf_join(&task[2], &ret[2]);
    printf("%s\n", (char *)ret[2]);
    lutf_exit(0);
    return 0;
}

// 百万级测试
static int _million(void) {
    assert(lutf_init() == 0);
// BUG: COUNT 取 4, 8, 12, 16 等数时 ret 最后一项无法正确输出
#define COUNT 4

    lutf_thread_t *threads =
        (lutf_thread_t *)malloc(COUNT * sizeof(lutf_thread_t));
    void **   ret = malloc(COUNT * sizeof(uint32_t));
    uint32_t *arg = (uint32_t *)malloc(COUNT * sizeof(uint32_t));
    for (size_t i = 0; i < COUNT; i++) {
        arg[i] = i;
        printf("arg: %d\n", arg[i]);
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        assert(lutf_create(&threads[i], test4, (void *)&arg[i]) == 0);
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        lutf_join(&threads[i], &ret[i]);
        printf("ret: %d\n", *(uint32_t *)ret[i]);
    }
    printf("Wait a second.\n");
    for (int i = 0; i < CLOCKS_PER_SEC * 500; i++)
        ;
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        assert(lutf_create(&threads[i], test4, (void *)&arg[i]) == 0);
    }
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        lutf_join(&threads[i], &ret[i]);
        printf("ret: %d\n", *(uint32_t *)ret[i]);
    }
    lutf_exit(0);
    return 0;
}

// 测试顺序
// create，join，exit，wait，self，equal，cancel，sync
int fifo(void) {
    printf("--------FIFO--------\n");
    printf("In this mode, threads are execute sequentially.\n");
    printf("----create----\n");
    printf("Create a thread and output its internal data.\n");
    printf("Functions used are: lutf_create.\n");
    assert(_create() == 0);
    printf("----join_exit----\n");
    printf("Create a thread, run and output its return value.\n");
    printf("Functions used are: lutf_init, lutf_create, lutf_join, "
           "lutf_exit.\n");
    assert(_join_exit() == 0);
    printf("----million----\n");
    printf("Create a million threads, run and output its return value.\n");
    printf("Functions used are: lutf_init, lutf_create, lutf_join, "
           "lutf_exit.\n");
    assert(_million() == 0);
    printf("--------FIFO END--------\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

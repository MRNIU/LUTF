
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// test_manage.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "unistd.h"
#include "lutf.h"

static int index = 5;

static void *test1(void *arg) {
    index = 5;
    while (1) {
        printf("test1 index %d\n", index--);
        if (index == 0) {
            printf("test1 cancel\n");
            lutf_cancel(lutf_self());
        }
    }
    printf("test1 ending\n");
    return NULL;
}

static void *test2(void *arg) {
    index = 5;
    while (1) {
        printf("test2 index %d\n", index--);
        if (index == 0) {
            printf("test2 cancel\n");
            lutf_cancel(lutf_self());
        }
    }
    printf("test2 ending\n");
    return NULL;
}

static void *test3(void *arg) {
    index = 5;
    while (1) {
        printf("test3 index %d\n", index--);
        if (index == 0) {
            printf("test3 cancel\n");
            lutf_cancel(lutf_self());
        }
    }
    printf("test3 ending\n");
    return NULL;
}

// 取消
static int cancel(void) {
    assert(lutf_init() == 0);
    lutf_thread_t task;
    assert(lutf_create(&task, test1, NULL) == 0);
    lutf_join(task, NULL);
    lutf_exit(0);
    return 0;
}

// 动态加入
static int join(void) {
    assert(lutf_init() == 0);
    lutf_thread_t task, task2;
    assert(lutf_create(&task, test2, NULL) == 0);
    assert(lutf_create(&task2, test3, NULL) == 0);
    lutf_join(task, NULL);
    lutf_join(task2, NULL);
    lutf_exit(0);
    return 0;
}

// 动态销毁
// 等待
// 同步互斥

// 进程管理测试
int test_manage(void) {
    printf("--------manage--------\n");
    assert(cancel() == 0);
    assert(join() == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif

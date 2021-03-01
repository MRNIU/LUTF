
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
#include "time.h"
#include "lutf.h"

static int index = 5;

static void *test1(void *arg __attribute__((unused))) {
    index = 3;
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

static void *test2(void *arg __attribute__((unused))) {
    index = 3;
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

static void *test3(void *arg __attribute__((unused))) {
    index = 3;
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

// 加入/取消/获取自身
static int join_cancel_self(void) {
    assert(lutf_init() == 0);
    lutf_thread_t task1, task2, task3;
    assert(lutf_create(&task1, test1, NULL) == 0);
    assert(lutf_create(&task2, test2, NULL) == 0);
    assert(lutf_create(&task3, test3, NULL) == 0);
    lutf_join(&task1, NULL);
    lutf_join(&task2, NULL);
    lutf_join(&task3, NULL);
    lutf_exit(0);
    return 0;
}

// 相等
static int equal(void) {
    assert(lutf_init() == 0);
    lutf_thread_t task1, task2;
    assert(lutf_create(&task1, test1, NULL) == 0);
    assert(lutf_create(&task2, test2, NULL) == 0);
    lutf_join(&task1, NULL);
    lutf_join(&task2, NULL);
    assert(lutf_equal(&task1, &task2) == 0);
    assert(lutf_equal(&task1, &task1) == 1);
    lutf_exit(0);
    return 0;
}

// 动态销毁
// 等待

// 同步互斥
#define SIZE 3
int          current = 0;
int          buffer[SIZE];
lutf_S_t *   empty;
lutf_S_t *   full;
lutf_S_t *   lock;
static void *producter(void *arg __attribute__((unused))) {
    int item;
    while (1) {
        item = clock();
        lutf_P(empty);
        lutf_P(lock);
        buffer[current] = item;
        current += 1;
        printf("Size:%d\n", current);
        lutf_V(lock);
        lutf_V(full);
        printf("Thread %d, product:\t%d\n", 666, item);
    }
    return NULL;
}

static void *consumer(void *arg __attribute__((unused))) {
    int item;
    while (1) {
        lutf_P(full);
        lutf_P(lock);
        current -= 1;
        item = buffer[current];
        lutf_V(lock);
        lutf_V(empty);
        printf("Thread %d, consume:\t%d\n", 233, item);
    }
    return NULL;
}
static int _sync(void) {
    lutf_thread_t threads[6];
    lutf_init();
    empty = lutf_createS(SIZE);
    full  = lutf_createS(0);
    lock  = lutf_createS(1);
    for (int i = 0; i < 4; i++) {
        assert(lutf_create(&threads[i], producter, NULL) == 0);
        lutf_join(&threads[i], NULL);
    }
    for (int i = 4; i < 6; i++) {
        assert(lutf_create(&threads[i], consumer, NULL) == 0);
        lutf_join(&threads[i], NULL);
    }
    // BUG
    // lutf_wait(&threads[5]);
    lutf_exit(0);
    return 0;
}

// 进程管理测试
int test_manage(void) {
    printf("--------manage--------\n");
    // assert(join_cancel_self() == 0);
    // assert(equal() == 0);
    // assert(_sync() == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif

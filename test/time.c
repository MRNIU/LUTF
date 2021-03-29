
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
static void *test1(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    for (size_t i = 0; i < C; i++) {
        if (i % 100000 == 0) {
            printf("test1");
        }
    }
    lutf_exit((void *)"This is test1 exit value");
    return NULL;
}

static void *test2(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    // Do some calculations
    for (size_t i = 0; i < C; i++) {
        if (i % 100000 == 0) {
            printf("test2");
        }
    }
    lutf_exit((void *)"This is test2 exit value");
    return NULL;
}

static void *test3(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    for (size_t i = 0; i < C; i++) {
        if (i % 100000 == 0) {
            printf("test3");
        }
    }
    lutf_exit((void *)"This is test3 exit value");
    return NULL;
}

static int _detach_exit_wait(void) {
    lutf_t *threads = malloc(3 * sizeof(lutf_t));
    char *  arg[3]  = {"This is test1 arg", "This is test2 arg",
                    "This is test3 arg"};
    assert(lutf_create(&threads[0], test1, (void *)arg[0]) == 0);
    assert(lutf_create(&threads[1], test2, (void *)arg[1]) == 0);
    assert(lutf_create(&threads[2], test3, (void *)arg[2]) == 0);
    // 开始运行
    lutf_detach(&threads[0]);
    lutf_detach(&threads[1]);
    lutf_detach(&threads[2]);
    // 等待退出
    lutf_wait(threads, 3);
    return 0;
}

#define BUF_SIZE 100
int buffer[BUF_SIZE];
#define FEE 1000
#define PROD 2
#define CONS 4
static int   current = 0;
lutf_S_t *   empty;
lutf_S_t *   full;
lutf_S_t *   lock;
static int   total_get = 0;
static void *producter(void *arg __attribute__((unused))) {
    int item;
    int i = 0;
    while (1) {
        item = i;
        i++;
        lutf_P(empty);
        lutf_P(lock);
        buffer[current] = item;
        current += 1;
        lutf_V(lock);
        lutf_V(full);
        // printf("p %p: %d, curr: %d\n", lutf_self(), item, current);
    }
    lutf_exit(NULL);
    return NULL;
}

static void *consumer(void *arg __attribute__((unused))) {
    int item;
    for (size_t i = 0; i < FEE; i++) {
        total_get++;
        lutf_P(full);
        lutf_P(lock);
        current -= 1;
        item = buffer[current];
        lutf_V(lock);
        lutf_V(empty);
        // printf("c %p: %d, curr: %d\n", lutf_self(), item, current);
    }
    lutf_exit(NULL);
    return NULL;
}

static int _sync(void) {
    lutf_t *p = malloc(PROD * sizeof(lutf_t));
    lutf_t *c = malloc(CONS * sizeof(lutf_t));
    empty     = lutf_createS(BUF_SIZE);
    full      = lutf_createS(0);
    lock      = lutf_createS(1);
    for (size_t i = 0; i < PROD; i++) {
        assert(lutf_create(&p[i], producter, NULL) == 0);
        lutf_detach(&p[i]);
    }
    for (size_t i = 0; i < CONS; i++) {
        assert(lutf_create(&c[i], consumer, NULL) == 0);
        lutf_detach(&c[i]);
    }
    lutf_wait(c, CONS);
    assert(total_get == FEE * CONS);
    return 0;
}

static void *test4(void *arg) {
    if (arg != NULL) {
        printf("arg: %d\n", *(uint32_t *)arg);
    }
    for (size_t i = 0; i < C; i++) {
        if (i % 100000 == 0) {
            printf("test4");
        }
    }
    lutf_exit(arg);
    return NULL;
}

static void *test5(void *arg) {
    if (arg != NULL) {
        printf("arg: %d\n", *(uint32_t *)arg);
    }
    // Do some calculations
    for (size_t i = 0; i < C; i++) {
        if (i % 100000 == 0) {
            printf("test5");
        }
    }
    lutf_exit(arg);
    return NULL;
}

static int _million(void) {
#define COUNT 10
    lutf_t *  threads = malloc(COUNT * sizeof(lutf_t));
    uint32_t *arg     = (uint32_t *)malloc(COUNT * sizeof(uint32_t));
    for (size_t i = 0; i < COUNT; i++) {
        arg[i] = i;
    }
    for (size_t i = 0; i < COUNT / 2; i++) {
        assert(lutf_create(&threads[i], test4, (void *)&arg[i]) == 0);
    }
    for (size_t i = COUNT / 2; i < COUNT; i++) {
        assert(lutf_create(&threads[i], test5, (void *)&arg[i]) == 0);
    }
    for (size_t i = 0; i < COUNT; i++) {
        lutf_detach(&threads[i]);
    }
    lutf_wait(threads, COUNT);
    return 0;
}

int time_(void) {
    printf("--------TIME--------\n");
    printf(
        "In this mode, threads will be scheduled based on time resources.\n");
    printf("----detach_exit_wait----\n");
    printf("Create a thread, run and output its return value.\n");
    printf("Functions used: lutf_create, lutf_detach, lutf_wait, lutf_exit.\n");
    assert(_detach_exit_wait() == 0);
    printf("----sync----\n");
    assert(_sync() == 0);
    printf("----million----\n");
    printf("Create a million threads, run and output its return value.\n");
    printf("Functions used are: lutf_create, lutf_detach, lutf_exit.\n");
    assert(_million() == 0);
    printf("--------TIME END--------\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

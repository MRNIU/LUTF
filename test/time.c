
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

#define C CLOCKS_PER_SEC
static size_t test1_count = 0;
static size_t test2_count = 0;
static size_t test3_count = 0;

static void *test1(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    for (size_t i = 0; i < C; i++) {
        ;
    }
    test1_count++;
    lutf_exit((void *)"This is test1 exit value");
    return NULL;
}

static void *test2(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    // Do some calculations
    for (size_t i = 0; i < C; i++) {
        ;
    }
    test2_count++;
    lutf_exit((void *)"This is test2 exit value");
    return NULL;
}

static void *test3(void *arg) {
    if (arg != NULL) {
        printf("arg: %s\n", (char *)arg);
    }
    for (size_t i = 0; i < C; i++) {
        ;
    }
    test3_count++;
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
    lutf_detach(&threads[0]);
    lutf_detach(&threads[1]);
    lutf_detach(&threads[2]);
    lutf_wait(threads, 3);
    assert(test1_count == 1);
    assert(test2_count == 1);
    assert(test3_count == 1);
    return 0;
}

#define BUF_SIZE 5000
int buffer[BUF_SIZE];
#define FEE 2000
#define PROD 2
#define CONS 4
static int   current = 0;
lutf_S_t *   empty;
lutf_S_t *   full;
lutf_S_t *   lock;
static int   total_get = 0;
static int   np        = 0;
static int   nc        = 0;
static void *producter(void *arg) {
    int item = 0;
    for (size_t i = 0; i < FEE * 2; i++) {
        item = i;
        lutf_P(empty);
        lutf_P(lock);
        buffer[current] = item;
        current += 1;
        lutf_V(lock);
        lutf_V(full);
    }
    np++;
    lutf_exit(arg);
    assert(0);
    return NULL;
}
static void *consumer(void *arg) {
    for (size_t i = 0; i < FEE; i++) {
        total_get++;
        lutf_P(full);
        lutf_P(lock);
        current -= 1;
        buffer[current] = buffer[current];
        lutf_V(lock);
        lutf_V(empty);
    }
    nc += 1;
    lutf_exit(arg);
    assert(0);
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
    assert(np == PROD);
    assert(nc == CONS);
    assert(total_get == FEE * CONS);
    return 0;
}

static size_t test4_count = 0;
static size_t test5_count = 0;

static void *test4(void *arg) {
    // Do some calculations
    for (size_t i = 0; i < C; i++) {
        ;
    }
    test4_count++;
    lutf_exit(arg);
    assert(0);
    return NULL;
}

static void *test5(void *arg) {
    // Do some calculations
    for (size_t i = 0; i < C; i++) {
        ;
    }
    test5_count++;
    lutf_exit(arg);
    assert(0);
    return NULL;
}

static int _million(void) {
#define COUNT 10000
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
    assert(test4_count == COUNT / 2);
    assert(test5_count == COUNT / 2);
    return 0;
}

int time_(void) {
    printf("--------TIME--------\n");
    printf("In this mode, threads will be scheduled based on time "
           "resources.\n");
    printf("----detach_exit_wait----\n");
    printf("Create a thread, run and output its return value.\n");
    printf("Functions used: lutf_create, lutf_detach, lutf_wait, "
           "lutf_exit.\n");
    assert(_detach_exit_wait() == 0);
    printf("----sync----\n");
    assert(_sync() == 0);
    printf("----million----\n");
    printf("Create %d threads, run and output its return value.\n", COUNT);
    printf("Functions used are: lutf_create, lutf_detach, lutf_exit.\n");
    assert(_million() == 0);
    printf("--------TIME END--------\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

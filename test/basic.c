
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

static int aaa = 5;

static void *test4(void *arg) {
    while (1) {
        printf("test4: %d\n", aaa);
        aaa--;
        sleep(1);
    }
    return NULL;
}

static void *test5(void *arg) {
    while (1) {
        printf("test5: %d\n", aaa);
        aaa++;
        sleep(1);
    }
    return NULL;
}

static void *test6(void *arg) {
    printf("test6\n");
    if (aaa == 0) {
        lutf_exit((void *)"aaa==0");
    }
    return NULL;
}

static int _create(void) {
    assert(lutf_init() == 0);
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
    lutf_exit(0);
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

static int _wait(void) {
    assert(lutf_init() == 0);
    lutf_thread_t task[3];
    void *        ret[3];
    assert(lutf_create(&task[0], test4, NULL) == 0);
    assert(lutf_create(&task[1], test5, NULL) == 0);
    assert(lutf_create(&task[2], test6, NULL) == 0);
    lutf_join(&task[0], NULL);
    lutf_join(&task[1], NULL);
    lutf_join(&task[2], &ret[2]);
    lutf_wait(&task[2]);
    printf("%s\n", (char *)ret[2]);
    lutf_exit(0);
    return 0;
}
static int _self(void) {
    return 0;
}
static int _equal(void) {
    return 0;
}
static int _cancel(void) {
    return 0;
}
static int _sync(void) {
    return 0;
}

// 测试顺序
// create，join，exit，wait，self，equal，cancel，sync
int basic(void) {
    printf("--------basic--------\n");
    printf("----create----\n");
    assert(_create() == 0);
    printf("----join_exit----\n");
    assert(_join_exit() == 0);
    printf("----wait----\n");
    // assert(_wait() == 0);
    printf("----self----\n");
    assert(_self() == 0);
    printf("----equal----\n");
    assert(_equal() == 0);
    printf("----cancel----\n");
    assert(_cancel() == 0);
    printf("----sync----\n");
    assert(_sync() == 0);
    // assert(arg_ret() == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif

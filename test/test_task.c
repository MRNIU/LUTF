
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// test_task.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "lutf.h"

static void *test1(void *argv __attribute__((unused))) {
    return NULL;
}

static void *test2(void *arg) {
    printf("test2:\narg: %s\n", (char *)arg);
    lutf_exit((void *)"test2 exit");
    return NULL;
}

// 线程内部信息
static int inter_info(void) {
    assert(lutf_init() == 0);
    lutf_thread_t task;
    assert(lutf_create(&task, test1, NULL) == 0);
    char *status[] = {
        "RUNNING", "EXIT", "WAIT", "SEM", "SLEEP",
    };
    printf("test1:\nid: %d, status: %s, func: %p, arg: %p, exit_value: %p, "
           "constext: "
           "%p, next: %p, waited: %p\n",
           task.id, status[task.status], task.func, task.arg, task.exit_value,
           task.context, task.next, task.waited);
    lutf_exit(0);
    return 0;
}

// 参数传递与返回值
static int arg_ret(void) {
    assert(lutf_init() == 0);
    lutf_thread_t task;
    void *        ret;
    char          arg[] = "test2 arg";
    assert(lutf_create(&task, test2, arg) == 0);
    lutf_join(task, &ret);
    printf("%s\n", (char *)ret);
    lutf_exit(0);
    return 0;
}

// 任务抽象测试
int test_task(void) {
    printf("--------task--------\n");
    assert(inter_info() == 0);
    assert(arg_ret() == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif

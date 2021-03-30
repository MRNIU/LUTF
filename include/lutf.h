
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// lutf.h for MRNIU/LUTF.

#ifndef _LUTF_H_
#define _LUTF_H_

#ifdef __cplusplus
extern "C" {
#endif

// 评审规则
// demo功能覆盖度
// 项目功能说明书
// 项目背景是否清晰、功能描述是否清晰、使用和操作指引是否清晰
// 题目完成的完整性和正确性
// 代码架构设计的优雅性
// 代码编写的规范性
// 测试用例完成度和覆盖程度

#include "stdint.h"
#include "sys/types.h"
#include "setjmp.h"

// do not less than lutf internal exec time, ms
#define SLICE (128)
// thread stack size, 128byte
#define LUTF_STACK_SIZE (16 * 1024 * 8)
// semaphore size
#define SEM_SIZE (1000)

// thread fun type
typedef void *(*lutf_fun_t)(void *);

typedef void *lutf_t;

// semaphore
typedef struct lutf_S {
    ssize_t  s;
    ssize_t  size;
    lutf_t **queue;
} lutf_S_t;

// prior
typedef enum {
    LOW  = 0,
    MID  = 1,
    HIGH = 2,
    NONE = 3,
} lutf_prior_t;

// thread status
typedef enum lutf_status {
    lutf_READY = 0,
    lutf_RUNNING,
    lutf_EXIT,
    lutf_WAIT,
    lutf_SEM,
    lutf_SLEEP,
} lutf_status_t;

// set prior
// thread: which thread
// p: prior
// return value: return 0 on success
int lutf_set_prior(lutf_t *thread, lutf_prior_t p);
// create a thread
// thread: thread structure
// fun: function will be exec
// argv: fun's parameters
// return value: return 0 on success
int lutf_create(lutf_t *thread, lutf_fun_t fun, void *arg);
// block the current thread and wait thread finish
// thread: which thread
// ret: thread return val
// return value: return 0 on success
int lutf_join(lutf_t *thread, void **ret);
// current thread exec concurrently with thread
// thread: which thread to be concurrent
// return value: return 0 on success
int lutf_detach(lutf_t *thread);
// wait thread array finish
// threads: array of threads to wait
// size: thread size
// return value: return 0 on success
int lutf_wait(lutf_t *threads, size_t size);
// exit a thread
// value: exit val
int lutf_exit(void *value);
// thread sleep
// thread: thread to sleep
// sec: sleep time, second
// return value: return 0 on success
int lutf_sleep(lutf_t *thread, size_t sec);
// get curr thread structure
// return value: current thread structure
lutf_t lutf_self(void);
// compare two threads if same
// return value：return 1 same
int lutf_equal(lutf_t *thread1, lutf_t *thread2);
// get thread status
// thread: which thread
// return value: one of lutf_status_t
lutf_status_t lutf_status(lutf_t *thread);
// cancel thread
// thread: thread to cancel
// return value: return 0 on success
int lutf_cancel(lutf_t *thread);
// create semaphore
lutf_S_t *lutf_createS(int ss);
// P
int lutf_P(lutf_S_t *s);
// V
int lutf_V(lutf_S_t *s);

#ifdef __cplusplus
}
#endif

#endif

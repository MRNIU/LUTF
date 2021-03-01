
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
// 项目能够提供可运行的demo
// demo可运行
// demo功能覆盖度
// 项目功能说明书
// 项目背景是否清晰、功能描述是否清晰、使用和操作指引是否清晰
// 项目代码
// 题目完成的完整性和正确性
// 代码架构设计的优雅性
// 代码编写的规范性
// 测试用例完成度和覆盖程度
// 晋级标准

// 评审专家根据复赛评审规则打分，分数排名前50的队伍将进入决赛。

// 整体逻辑
// 在初始化时会创建一个定时器，该定时器只触发一次，触发后跳进信号处理
// 在信号处理函数中，进行 curr_thread 的设置，同时更新 thread 的运行时间信息，
// 之后信号处理函数会重新设置一个定时器，只触发一次，在最后调用 longjmp，跳转到
// curr_thread，执行函数
// 在这之前需要调用 run 函数，

// 优化方向
// TODO:
// 进程 id 的分配，避免冲突
// 其它调度方式的支持
// 确保 FIFO
// 任务的等待
// 运行中某个任务出现问题如何处理？
// 在 FIFO 的情况下如果不进行任务切换会导致剩余的所有任务被阻塞
// stack 的处理是否必要？

#include "stdint.h"
#include "sys/types.h"
#include "setjmp.h"

// 时间片 ms
#define SLICE (300)
// 线程栈大小
#define LUTF_STACK_SIZE (4096)
//信号量数量
#define SEM_SIZE (1000)

// 线程状态
typedef enum lutf_status {
    lutf_RUNNING = 0,
    lutf_EXIT,
    lutf_WAIT,
    lutf_SEM,
    lutf_SLEEP,
} lutf_status_t;

// 线程函数类型
typedef void *(*lutf_fun_t)(void *);

// 线程 id 类型
typedef int32_t lutf_task_id_t;

// 线程类型
typedef struct lutf_thread {
    // 线程 id
    lutf_task_id_t id;
    // 线程状态
    lutf_status_t status;
    // 线程函数
    lutf_fun_t func;
    // 线程参数
    void *arg;
    // 线程退出值
    void *exit_value;
    // 线程上下文
    jmp_buf context;
    // 上一个线程
    struct lutf_thread *prev;
    // 下一个线程
    struct lutf_thread *next;
    // 等待队列
    struct lutf_thread *waited;
} lutf_thread_t;

// 信号量
typedef struct lutf_S {
    long            s;
    long            size;
    lutf_thread_t **queue;
} lutf_S_t;

// 全局状态
typedef struct lutf_env {
    // 有 n 个线程
    size_t nid;
    // main 线程
    lutf_thread_t *main_thread;
    // 当前线程
    lutf_thread_t *curr_thread;
} lutf_env_t;

// LUTF 初始化
// 返回值：成功返回 0
int lutf_init(void);
// 线程创建
// thread: 线程结构
// fun: 要执行的函数
// argv: fun 的参数
// 返回值：成功返回 0
int lutf_create(lutf_thread_t *thread, lutf_fun_t fun, void *arg);
// 等待 thread 结束
// thread: 要等待的线程
// ret: 线程返回值
// 返回值：lutf_join 函数执行情况，成功返回 0
int lutf_join(lutf_thread_t *thread, void **ret);
// 线程退出
// value: 退出参数
int lutf_exit(void *value);
// 等待线程执行完毕
int lutf_wait(lutf_thread_t *thread);
// 获取当前线程结构
// 返回值：当前线程结构
lutf_thread_t *lutf_self(void);
// 比较两个线程是否相同
// 返回值：相同返回 1
int lutf_equal(lutf_thread_t *thread1, lutf_thread_t *thread2);
// 取消线程
// thread: 要取消的线程
// 返回值：成功返回 0
int lutf_cancel(lutf_thread_t *thread);
// 创建信号量
lutf_S_t *lutf_createS(int ss);
// P
int lutf_P(lutf_S_t *s);
// V
int lutf_V(lutf_S_t *s);

#ifdef __cplusplus
}
#endif

#endif

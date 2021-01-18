
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

// 线程状态
typedef enum lutf_status {
    lutf_RUNNING = 0,
    lutf_EXIT,
    lutf_WAIT,
    lutf_SLEEP,
} lutf_status_t;

// 线程函数类型
typedef void *(*lutf_fun_t)(void *);

// 线程 id 类型
typedef int32_t lutf_task_id_t;

// 错误码类型
typedef enum RETCODE {
    // 成功
    SUCCESS = 0,
    FAIL    = -1,
} RETCODE_t;

// 优先级类型
typedef enum PRIO {
    HIGH = 3,
    MID  = 2,
    LOW  = 1,
} PRIO_t;

// 线程类型
typedef struct lutf_thread {
    // 线程 id
    lutf_task_id_t id;
    // 线程优先级
    PRIO_t prio;
    // 线程状态
    lutf_status_t status;
    // 线程栈
    char *stack;
    // 线程函数
    lutf_fun_t func;
    // 线程参数
    void *argv;
    // 线程返回值
    void *ret;
    // 线程退出状态
    RETCODE_t exit_code;
    // 线程上下文
    jmp_buf context;
    // 下一个线程
    struct lutf_thread *next;
} lutf_thread_t;

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
// 返回当前函数的指针
lutf_thread_t *lutf_init(void);
// 线程创建
// fun: 要执行的函数
// argv: fun 的参数
int lutf_create_task(lutf_thread_t *thread, lutf_fun_t fun, void *argv);
// 将线程加入调度
// thread: 要加入的线程
RETCODE_t lutf_run(lutf_thread_t *thread);
// 线程删除
// thread: 要删除的线程
RETCODE_t lutf_del_task(lutf_thread_t *thread);
// 线程退出
// status: 退出状态
RETCODE_t lutf_exit_task(int32_t status);
// 设置线程优先级
// prio: 指定优先级
RETCODE_t lutf_set_task_prio(PRIO_t prio);
// 获取线程 id
// thread: 要获取 id 的线程
lutf_task_id_t lutf_get_task_id(lutf_thread_t *thread);
// 获取线程状态
// thread: 要获取状态的线程
lutf_status_t lutf_get_task_status(lutf_thread_t *thread);
// 等待线程执行完毕
int lutf_wait(lutf_thread_t *thread);

#ifdef __cplusplus
}
#endif

#endif


// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// lutf.h for MRNIU/LUTF.

#ifndef _LUTF_H_
#define _LUTF_H_

#ifdef __cplusplus
extern "C" {
#endif

// 优化方向
// TODO:
// 避免 setjmp 嵌套
// 确保函数执行完成后回收占用的资源

#include "stdint.h"
#include "sys/types.h"
#include "setjmp.h"

// 时间片 100ms
#define SLICE (100)
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
    // 信号处理函数注册失败
    ALARM_HANDLER_ERR = -1,
    // 内存不足
    NO_ENOUGH_MEM = -2,
    // 优先队列创建失败
    PRIO_QUEUE_NEW_FAILED = -3,
    // 优先队列插入失败
    PRIO_QUEUE_INSERT_FAILED = -4,
    // 计时器错误
    TIMER_ERR = -5,
} RETCODE_t;

// 调度算法类型
typedef enum SCHED_ALG {
    // 优先队列
    PRIO_QUEUE,
} SCHED_ALG_t;

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
} lutf_thread_t;

// 全局状态
typedef struct lutf_env {
    // 有 n 个线程
    size_t nid;
    // 调度算法
    SCHED_ALG_t alg;
    // 任务管理结构
    void *tasks;
    // 当前线程
    lutf_thread_t *curr_thread;
    // 当前线程剩余时间
    useconds_t time_remaining;

} lutf_env_t;

// LUTF 初始化
// alg: 要使用的算法
RETCODE_t lutf_init(SCHED_ALG_t alg);
// 线程创建
// fun: 要执行的函数
// argv: fun 的参数
lutf_thread_t *lutf_create_task(lutf_fun_t fun, void *argv);
// 将线程加入调度
// thread: 要加入的线程
RETCODE_t lutf_add_sched(lutf_thread_t *thread);
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
// 开始调度
void    lutf_sched_start(void);
void    lutf_sched_end(void);
uint8_t lutf_get_sched_status(void);

#ifdef __cplusplus
}
#endif

#endif


// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// lutf.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#include "signal.h"
#include "stdlib.h"

#include "sys/time.h"
#include "lutf.h"

// 全局状态
static lutf_env_t env = {
    .nid         = 0,
    .main_thread = NULL,
    .curr_thread = NULL,
};

struct itimerval tick_once = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = SLICE,
};

struct itimerval tick_cancel = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = 0,
};

// 时钟信号处理
static void sig_alarm_handler(int signo) {
    if (signo != SIGALRM) {
        printf("sig_alarm_handler err!\n");
        return;
    }
    if (setjmp(env.curr_thread->context) == 0) {
        do {
            // 切换到下个线程
            env.curr_thread = env.curr_thread->next;
            // 根据状态
            switch (env.curr_thread->status) {
                // 跳过
                case lutf_RUNNING: {
                    printf("RUNNING\n");
                    break;
                }
                // 跳过
                case lutf_EXIT: {
                    lutf_del_task(env.curr_thread);
                    printf("EXIT\n");
                    break;
                }
                case lutf_WAIT: {
                    printf("WAIT\n");
                    break;
                }
                case lutf_SLEEP: {
                    break;
                }
            }
            // 循环直到 RUNNING 状态的线程
        } while (env.curr_thread->status != lutf_RUNNING);
        // 开启 timer
        int ret = setitimer(ITIMER_REAL, &tick_once, NULL);
        if (ret != 0) {
            printf("sched timer err\n");
        }
        // 开始执行
        // 会返回到 lutf_run 处的 setjmp
        longjmp(env.curr_thread->context, 1);
    }
    return;
}

RETCODE_t lutf_init(void) {
    int ret = 0;
    // 注册信号处理函数
    if (signal(SIGALRM, sig_alarm_handler) == SIG_ERR) {
        printf("init signal() err!\n");
        return FAIL;
    }
    // 初始化 main 线程信息
    lutf_thread_t *thread_main = (lutf_thread_t *)malloc(sizeof(lutf_thread_t));
    if (thread_main == NULL) {
        printf("init malloc err!\n");
        return FAIL;
    }
    // 线程 id 为 0
    thread_main->id = 0;
    // 默认优先级
    thread_main->prio = MID;
    // 状态为正在运行
    thread_main->status = lutf_RUNNING;
    // main 线程使用默认栈
    thread_main->stack = NULL;
    // 函数指针为 NULL
    thread_main->func = NULL;
    // 参数为 NULL
    // 设为 0xCDCD 便于调试
    thread_main->argv = 0xCDCD;
    // 返回值为空
    thread_main->ret = NULL;
    // 返回状态默认为成功
    thread_main->exit_code = 0;
    // 初始化链表
    thread_main->next = thread_main;
    // 更新全局信息
    env.nid         = 1;
    env.main_thread = thread_main;
    env.curr_thread = thread_main;

    // 开启 timer
    ret = setitimer(ITIMER_REAL, &tick_once, NULL);
    if (ret != 0) {
        printf("init setitimer() err!\n");
        return FAIL;
    }
    return SUCCESS;
}

lutf_thread_t *lutf_create_task(lutf_fun_t fun, void *argv) {
    // 分配线程空间
    lutf_thread_t *thread = (lutf_thread_t *)malloc(sizeof(lutf_thread_t));
    if (thread == NULL) {
        printf("lutf_create_task thread NULL.\n");
        return NULL;
    }
    // 分配线程栈
    thread->stack = (char *)malloc(LUTF_STACK_SIZE);
    if (thread->stack == NULL) {
        printf("lutf_create_task stack NULL.\n");
        return NULL;
    }
    // 线程 id 线性增加
    thread->id = env.nid;
    // 默认优先级
    thread->prio = MID;
    // 设置为 READY
    thread->status = lutf_RUNNING;
    // 要执行的函数指针
    thread->func = fun;
    // 参数
    thread->argv = argv;
    // 返回值
    thread->ret = NULL;
    // 退出代码
    thread->exit_code = 0;
    thread->next      = thread;
    return thread;
}

RETCODE_t lutf_run(lutf_thread_t *thread) {
    int ret = 0;
    // 添加到线程管理结构
    thread->next          = env.main_thread->next;
    env.main_thread->next = thread;
    // 更新全局信息
    env.nid += 1;

    // 初始化 thread 的上下文
    // 如果不是从 thread 返回，即还没有运行
    if (setjmp(thread->context) == 0) {
        // 将状态更改为 RUNNING
        thread->status = lutf_RUNNING;
        // 等待执行
        if (setjmp(env.curr_thread->context) == 0) {
            // 将当前线程设为 thread
            env.curr_thread = thread;
            longjmp(thread->context, 1);
        }
        else {
            return SUCCESS;
        }
    }
    // 如果 setjmp 返回值不为 0，说明是从 thread 返回，
    // 这时 env->curr_thread 指向新的线程
    else {
        // 取消所有定时器，这样可以保证线程的 FIFO
        // 缺点是如果当前线程执行时间过长，lutf 无法利用这段时间
        ret = setitimer(ITIMER_REAL, &tick_cancel, NULL);
        if (ret != 0) {
            printf("lutf_run setitimer() err!\n");
            return FAIL;
        }
        // 手动切换线程栈
#ifdef __x86_64__
        __asm__("mov %0, %%rsp"
                :
                : "r"(env.curr_thread->stack + LUTF_STACK_SIZE));
#endif
        // 执行函数
        env.curr_thread->func(env.curr_thread->argv);
        env.curr_thread->status = lutf_EXIT;
        raise(SIGALRM);
    }
    return SUCCESS;
}

RETCODE_t lutf_del_task(lutf_thread_t *thread) {
    lutf_thread_t *pre;
    pre = thread;
    // 如果是最后一个线程，说明是 main thread，直接返回
    if (pre == pre->next) {
        return SUCCESS;
    }
    // 找到前置节点
    while (pre->next != thread) {
        pre = pre->next;
    }
    // 设置指针
    pre->next = thread->next;
    free(thread->stack);
    free(thread);
    return SUCCESS;
}

#ifdef __cplusplus
}
#endif

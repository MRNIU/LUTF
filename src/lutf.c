
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
#include "assert.h"
#include "sys/time.h"
#include "lutf.h"

// TODO: 使用 sigaction 替换 signal

// 全局状态
static lutf_env_t env = {
    .nid         = 0,
    .main_thread = NULL,
    .curr_thread = NULL,
};

// 仅触发一次定时器
struct itimerval tick_once = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = SLICE,
};

// 取消所有定时
struct itimerval tick_cancel = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = 0,
};

static int _wait(lutf_thread_t *thread) {
    lutf_thread_t *p;
    p = thread;
    do {
        p = p->next;
        if (*p->waited == thread) {
            if (p->status == lutf_EXIT) {
                thread->status = lutf_RUNNING;
                break;
            }
        }
    } while (p != thread);
    return 0;
}

// 时钟信号处理
static void sig_alarm_handler(int signo) {
    if (setjmp(env.curr_thread->context) == 0) {
        do {
            // 切换到下个线程
            env.curr_thread = env.curr_thread->next;
            if (env.curr_thread == env.main_thread) {
                break;
            }
            // 根据状态
            switch (env.curr_thread->status) {
                // 跳过
                case lutf_RUNNING: {
                    printf("RUNNING\n");
                    break;
                }
                case lutf_WAIT: {
                    _wait(env.curr_thread);
                    break;
                }
                case lutf_SLEEP: {
                    break;
                }
                // 跳过
                case lutf_EXIT: {
                }
                case lutf_SEM: {
                }
            }
            // 循环直到 RUNNING 状态的线程
        } while (env.curr_thread->status != lutf_RUNNING);
        // 开启 timer
        assert(setitimer(ITIMER_REAL, &tick_once, NULL) == 0);
        // 开始执行
        // 会返回到 lutf_join 处的 setjmp
        longjmp(env.curr_thread->context, 1);
    }
    return;
}

int lutf_init(void) {
    // 注册信号处理函数
    signal(SIGALRM, sig_alarm_handler);
    // 初始化 main 线程信息
    lutf_thread_t *thread_main = (lutf_thread_t *)malloc(sizeof(lutf_thread_t));
    assert(thread_main != NULL);
    // 线程 id 为 0
    thread_main->id = 0;
    // 状态为正在运行
    thread_main->status = lutf_RUNNING;
    // 函数指针为 NULL
    thread_main->func = NULL;
    // 参数设为 0xCDCD, 便于调试
    thread_main->arg = (void *)0xCDCD;
    // 返回值默认为空
    thread_main->exit_value = NULL;
    // 初始化链表
    thread_main->next = thread_main;
    // 更新全局信息
    env.nid         = 1;
    env.main_thread = thread_main;
    env.curr_thread = thread_main;
    // 开启 timer
    assert(setitimer(ITIMER_REAL, &tick_once, NULL) == 0);
    return 0;
}

int lutf_create(lutf_thread_t *thread, lutf_fun_t fun, void *arg) {
    assert(thread != NULL);
    // 线程 id 线性增加
    thread->id = env.nid;
    // 设置为 READY
    thread->status = lutf_RUNNING;
    // 要执行的函数指针
    thread->func = fun;
    // 参数
    thread->arg = arg;
    // 退出值
    thread->exit_value = NULL;
    thread->next       = thread;
    return 0;
}

int lutf_join(lutf_thread_t thread, void **ret) {
    // 添加到线程管理结构
    thread.next           = env.main_thread;
    env.main_thread->prev = &thread;
    // 更新全局信息
    env.nid += 1;

    // 初始化 thread 的上下文
    // 如果不是从 thread 返回，即还没有运行
    if (setjmp(thread.context) == 0) {
        // 将状态更改为 RUNNING
        thread.status = lutf_RUNNING;
        // 等待执行
        if (setjmp(env.curr_thread->context) == 0) {
            // 将当前线程设为 thread
            env.curr_thread = &thread;
            longjmp(thread.context, 1);
        }
        else {
            return 0;
        }
    }
    // 如果 setjmp 返回值不为 0，说明是从 thread 返回，
    // 这时 env->curr_thread 指向新的线程
    else {
        // 取消所有定时器，这样可以保证线程的 FIFO
        // 缺点是如果当前线程执行时间过长，lutf 无法利用这段时间
        assert(setitimer(ITIMER_REAL, &tick_cancel, NULL) == 0);
        // 执行函数
        env.curr_thread->func(env.curr_thread->arg);
        if (ret != NULL) {
            *ret = env.curr_thread->exit_value;
        }
        env.curr_thread->status = lutf_EXIT;
        raise(SIGALRM);
    }
    return 0;
}

// TODO: main 退出时将 SIGALRM 处理恢复默认
int lutf_exit(void *value) {
    // 如果只剩 main 线程，结束 lutf
    if (env.curr_thread == env.main_thread) {
        assert(setitimer(ITIMER_REAL, &tick_cancel, NULL) == 0);
        // 释放 main 信息占用空间
        free(env.curr_thread);
    }
    else {
        env.curr_thread->exit_value = value;
        env.curr_thread->status     = lutf_EXIT;
    }
    return 0;
}

int lutf_wait(lutf_thread_t *thread) {
    // 停止定时器
    assert(setitimer(ITIMER_REAL, &tick_cancel, NULL));
    env.curr_thread->status = lutf_WAIT;
    *thread->waited         = env.curr_thread;
    raise(SIGALRM);
    return thread->status;
}

lutf_thread_t *lutf_self(void) {
    return env.curr_thread;
}

int lutf_cancel(lutf_thread_t *thread) {
    lutf_exit(NULL);
    raise(SIGALRM);
    return 0;
}

lutf_S_t *lutf_createS(int ss) {
    lutf_S_t *s;
    s = malloc(sizeof(lutf_S_t));
    assert(s != NULL);
    s->s     = ss;
    s->size  = SEM_SIZE;
    s->queue = malloc(sizeof(lutf_thread_t *) * s->size);
    return s;
}

int lutf_P(lutf_S_t *s) {
    if (s->s > 0) {
        s->s -= 1;
        assert(setitimer(ITIMER_REAL, &tick_cancel, NULL) == 0);
    }
    else {
        s->s -= 1;
        if (s->size < labs(s->s)) {
            s->size += SEM_SIZE;
            s->queue = realloc(s->queue, sizeof(lutf_thread_t *) * s->size);
        }
        env.curr_thread->status  = lutf_SEM;
        s->queue[labs(s->s) - 1] = env.curr_thread;
        raise(SIGALRM);
    }
    return 0;
}

int lutf_V(lutf_S_t *s) {
    if (s->s >= 0) {
        s->s += 1;
    }
    else {
        s->s += 1;
        s->queue[labs(s->s)]->status = lutf_RUNNING;
    }
    assert(setitimer(ITIMER_REAL, &tick_cancel, NULL) == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif

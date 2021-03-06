
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
#include "time.h"
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

// 用于获取剩余时间
struct itimerval old_tick = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = 0,
};
// TIME 方式下使用
struct itimerval tick_low = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = SLICE / 2,
};
struct itimerval tick_mid = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = SLICE,
};

struct itimerval tick_high = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = 2 * SLICE,
};

// 信号处理
struct sigaction sig_act, sig_oact;
// 与时钟信号配合
sigset_t newmask, oldmask, suspmask;

typedef enum {
    ERR_FIFO,
    ERR_TIME,
} err_t;

static void err_handle(err_t no) {
    switch (no) {
        case ERR_FIFO: {
            printf("You need in FIFO mode.\n");
            break;
        }
        case ERR_TIME: {
            printf("You need in TIME mode.\n");
            break;
        }
        default: {
            break;
        }
    }
    return;
}

// thread 要等待其 wait 队列为空才能退出
static int _wait(lutf_thread_t *thread) {
    // lutf_thread_t *p    = thread->wait;
    // int            flag = 1;
    // printf("1\n");
    // for (size_t i = 0; i < thread->wait_num; i++) {
    //     if (p[i].status != lutf_EXIT) {
    //         flag = 0;
    //     }
    // }
    // if (flag) {
    //     thread->status = lutf_RUNNING;
    // }
    lutf_thread_t *p;
    p = thread;
    do {
        p = p->next;
        if (p->waited == thread) {
            if (p->status == lutf_EXIT) {
                thread->status = lutf_RUNNING;
                break;
            }
        }
    } while (p != thread);
    return 0;
}

// 时钟信号处理
static void sig_alarm_handler(int signo __attribute__((unused))) {
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
                case lutf_READY: {
                    printf("READY\n");
                    break;
                }
                case lutf_RUNNING: {
                    printf("RUNNING\n");
                    break;
                }
                case lutf_WAIT: {
                    printf("WAIT\n");
                    _wait(env.curr_thread);
                    break;
                }
                case lutf_SLEEP: {
                    if (clock() > env.curr_thread->resume_time) {
                        env.curr_thread->status = lutf_RUNNING;
                    }
                    break;
                }
                // 跳过
                case lutf_EXIT: {
                    printf("EXIT\n");
                }
                case lutf_SEM: {
                }
            }
            // 循环直到 RUNNING 状态的线程
        } while (env.curr_thread->status != lutf_RUNNING);
        if (env.sched_method == TIME) {
            // 根据优先级调整运行时间
            switch (env.curr_thread->prior) {
                case LOW: {
                    // 开启 timer
                    assert(setitimer(ITIMER_REAL, &tick_low, NULL) == 0);
                    break;
                }
                case MID: {
                    // 开启 timer
                    assert(setitimer(ITIMER_REAL, &tick_mid, NULL) == 0);
                    break;
                }
                case HIGH: {
                    // 开启 timer
                    assert(setitimer(ITIMER_REAL, &tick_high, NULL) == 0);
                    break;
                }
            }
            // 获取剩余时间
            // getitimer(ITIMER_REAL, &left);
        }
        printf("env.curr_thread->id: %d\n", env.curr_thread->id);
        printf("env.curr_thread->prev->id: %d\n", env.curr_thread->prev->id);
        printf("env.curr_thread->next->id: %d\n", env.curr_thread->next->id);
        // 开始执行
        // 会返回到 lutf_join 处的 setjmp
        longjmp(env.curr_thread->context, 1);
    }
    return;
}

// 构造函数，在 main 之前执行
__attribute__((constructor)) static void init(void) {
    printf("init\n");
    // 初始化 main 线程信息
    lutf_thread_t *thread_main = (lutf_thread_t *)malloc(sizeof(lutf_thread_t));
    assert(thread_main != NULL);
    // 线程 id 为 0
    thread_main->id = 0;
    // 状态为正在运行
    thread_main->status = lutf_RUNNING;
    // 函数指针为 NULL
    thread_main->func = NULL;
    // 参数设为 NULL
    thread_main->arg = NULL;
    // 返回值默认为空
    thread_main->exit_value = NULL;
    // 初始化链表
    thread_main->prev   = thread_main;
    thread_main->next   = thread_main;
    thread_main->waited = NULL;
    // 优先级默认低
    thread_main->prior       = MID;
    thread_main->resume_time = 0;
    // 更新全局信息
    env.nid         = 1;
    env.main_thread = thread_main;
    env.curr_thread = thread_main;
    // 默认调度方式
    env.sched_method = FIFO;
    // 手动调用调度函数
    sig_alarm_handler(SIGALRM);
    return;
}

// 析构函数，在 main 结束后执行
__attribute__((destructor)) static void finit(void) {
    // 如果只剩 main 线程，结束 lutf
    if (env.main_thread != NULL) {
        if (env.sched_method == TIME) {
            // 取消时钟
            assert(setitimer(ITIMER_REAL, &tick_cancel, NULL) == 0);
            // 恢复之前的系统默认信号和默认信号处理。
            sigaction(SIGALRM, &sig_oact, NULL);
            sigprocmask(SIG_SETMASK, &oldmask, NULL);
        }
        // 释放 main 信息占用空间
        free(env.main_thread);
    }
    printf("finit\n");
    return;
}

int lutf_set_sched(lutf_sched_t method) {
    // 要设置的与正在用的相同
    if (env.sched_method == method) {
        // 直接返回
        return 0;
    }
    // 之前是 TIME，现在换成 FIFO
    else if (method == FIFO) {
        env.sched_method = FIFO;
        // 取消定时器
        assert(setitimer(ITIMER_REAL, &tick_cancel, NULL) == 0);
        // 将 SIGALRM 重置为默认
        sig_act.sa_handler = SIG_DFL;
        sigemptyset(&sig_act.sa_mask);
        sig_act.sa_flags = SA_RESETHAND;
        sigaction(SIGALRM, &sig_act, 0);
        // 手动调用调度函数
        sig_alarm_handler(SIGALRM);
    }
    // 之前是 FIFO，现在换成 TIME
    else if (method == TIME) {
        env.sched_method = TIME;
        // 注册信号处理函数
        // signal(SIGALRM, sig_alarm_handler);
        sig_act.sa_handler = sig_alarm_handler;
        sig_act.sa_flags   = 0;
        sigemptyset(&sig_act.sa_mask);
        // 注册信号捕捉函数。
        sigaction(SIGALRM, &sig_act, &sig_oact);
        // 初始化
        // sigemptyset(&newmask);
        // sigaddset(&newmask, SIGALRM);
        // 将这个信号加入到屏蔽字当中。
        // sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        // 开启时钟
        assert(setitimer(ITIMER_REAL, &tick_once, NULL) == 0);
        // suspmask = oldmask;
        // 确保suspmask中没有屏蔽SIGALRM信号。
        // sigdelset(&suspmask, SIGALRM);
        // 用suspmask去替换block表，从而临时解除对SIGALRM信号的阻塞
        // sigsuspend(&suspmask);
    }
    return 0;
}

int lutf_set_prior(lutf_thread_t *thread, lutf_prior_t p) {
    err_handle(ERR_TIME);
    thread->prior = p;
    return 0;
}

int lutf_create(lutf_thread_t *thread, lutf_fun_t fun, void *arg) {
    assert(thread != NULL);
    // id 默认为 -1
    thread->id = -1;
    // 设置为 READY
    thread->status = lutf_READY;
    // 要执行的函数指针
    thread->func = fun;
    // 参数
    thread->arg = arg;
    // 退出值
    thread->exit_value  = NULL;
    thread->prev        = thread;
    thread->next        = thread;
    thread->waited      = NULL;
    thread->prior       = LOW;
    thread->resume_time = 0;
    return 0;
}

int lutf_join(lutf_thread_t *thread, void **ret) {
    // 添加到线程管理结构
    lutf_thread_t *prev      = env.main_thread->prev;
    lutf_thread_t *next      = env.main_thread;
    lutf_thread_t *new_entry = thread;

    prev->next      = new_entry;
    next->prev      = new_entry;
    new_entry->prev = prev;
    new_entry->next = next;

    // 设置 id
    thread->id = env.nid;
    // 更新全局信息
    env.nid += 1;

    // env.main_thread->status = lutf_WAIT;

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
            return 0;
        }
    }
    // 如果 setjmp 返回值不为 0，说明是从 thread 返回，
    // 这时 env->curr_thread 指向新的线程
    else {
        // 执行函数
        env.curr_thread->func(env.curr_thread->arg);
        if (ret != NULL) {
            *ret = env.curr_thread->exit_value;
        }
        env.curr_thread->status = lutf_EXIT;
        if (env.sched_method == TIME) {
            assert(setitimer(ITIMER_REAL, &tick_cancel, NULL) == 0);
            raise(SIGALRM);
        }
        else if (env.sched_method == FIFO) {
            sig_alarm_handler(SIGALRM);
        }
    }
    return 0;
}

int lutf_exit(void *value) {
    if (env.curr_thread != env.main_thread) {
        env.curr_thread->exit_value = value;
        env.curr_thread->status     = lutf_EXIT;
    }
    return 0;
}

int lutf_wait(lutf_thread_t *thread) {
    assert(env.sched_method == TIME);
    // 停止定时器
    assert(setitimer(ITIMER_REAL, &tick_cancel, NULL) == 0);
    env.curr_thread->status = lutf_WAIT;
    // 记录原有 wait 线程表
    // env.curr_thread->wait_num++;
    // env.curr_thread->wait =
    //     realloc(env.curr_thread->wait,
    //             sizeof(lutf_thread_t *) * env.curr_thread->wait_num);
    thread->waited = env.curr_thread;
    raise(SIGALRM);
    return 0;
}

int lutf_sleep(lutf_thread_t *thread, size_t sec) {
    err_handle(ERR_TIME);
    thread->status      = lutf_SLEEP;
    thread->resume_time = clock() + sec * CLOCKS_PER_SEC;
    raise(SIGALRM);
    return 0;
}

lutf_thread_t *lutf_self(void) {
    return env.curr_thread;
}

int lutf_equal(lutf_thread_t *thread1, lutf_thread_t *thread2) {
    if (thread1->id != thread2->id) {
        return 0;
    }
    if (thread1->status != thread2->status) {
        return 0;
    }
    if (thread1->func != thread2->func) {
        return 0;
    }
    if (thread1->arg != thread2->arg) {
        return 0;
    }
    if (thread1->exit_value != thread2->exit_value) {
        return 0;
    }
    if (thread1->context != thread2->context) {
        return 0;
    }
    if (thread1->prev != thread2->prev) {
        return 0;
    }
    if (thread1->next != thread2->next) {
        return 0;
    }
    if (thread1->waited != thread2->waited) {
        return 0;
    }
    return 1;
}

int lutf_cancel(lutf_thread_t *thread) {
    lutf_exit(NULL);
    raise(SIGALRM);
    return 0;
}

lutf_S_t *lutf_createS(int ss) {
    err_handle(ERR_TIME);
    lutf_S_t *s;
    s = malloc(sizeof(lutf_S_t));
    assert(s != NULL);
    s->s     = ss;
    s->size  = SEM_SIZE;
    s->queue = malloc(sizeof(lutf_thread_t *) * s->size);
    return s;
}

int lutf_P(lutf_S_t *s) {
    err_handle(ERR_TIME);
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
    err_handle(ERR_TIME);
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

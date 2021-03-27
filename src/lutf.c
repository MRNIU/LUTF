
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

static lutf_env_t env = {
    .nid         = 0,
    .main_thread = NULL,
    .curr_thread = NULL,
};

struct itimerval tick_cancel = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = 0,
};

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

struct itimerval *itimer[3] = {&tick_low, &tick_mid, &tick_high};

struct sigaction sig_act;

static inline int list_free(lutf_entry_t *list) {
    lutf_entry_t *entry;
    entry = list;
    while (entry != NULL) {
        lutf_entry_t *next;
        next = entry->next;
        free(entry);
        entry = next;
    }
    return 0;
}

static inline lutf_entry_t *list_prepend(lutf_entry_t **list,
                                         lutf_thread_t *data) {
    lutf_entry_t *newentry;
    if (list == NULL) {
        return NULL;
    }
    newentry = malloc(sizeof(lutf_entry_t));
    if (newentry == NULL) {
        return NULL;
    }
    newentry->data = data;
    if (*list != NULL) {
        (*list)->prev = newentry;
    }
    newentry->prev = NULL;
    newentry->next = *list;
    *list          = newentry;
    return newentry;
}

static inline lutf_entry_t *list_append(lutf_entry_t **list,
                                        lutf_thread_t *data) {
    lutf_entry_t *rover;
    lutf_entry_t *newentry;
    if (list == NULL) {
        return NULL;
    }
    newentry = malloc(sizeof(lutf_entry_t));
    if (newentry == NULL) {
        return NULL;
    }
    newentry->data = data;
    newentry->next = NULL;
    if (*list == NULL) {
        *list          = newentry;
        newentry->prev = NULL;
    }
    else {
        for (rover = *list; rover->next != NULL; rover = rover->next) {
            ;
        }
        newentry->prev = rover;
        rover->next    = newentry;
    }
    return newentry;
}

static inline lutf_entry_t *list_prev(lutf_entry_t *listentry) {
    if (listentry == NULL) {
        return NULL;
    }
    return listentry->prev;
}

static inline lutf_entry_t *list_next(lutf_entry_t *listentry) {
    if (listentry == NULL) {
        return NULL;
    }
    return listentry->next;
}

static inline lutf_thread_t *list_data(lutf_entry_t *listentry) {
    if (listentry == NULL) {
        return NULL;
    }
    return listentry->data;
}

static inline lutf_entry_t *list_nth_entry(lutf_entry_t *list, unsigned int n) {
    lutf_entry_t *entry;
    unsigned int  i;
    entry = list;
    for (i = 0; i < n; ++i) {
        if (entry == NULL) {
            return NULL;
        }
        entry = entry->next;
    }
    return entry;
}

static inline lutf_thread_t *list_nth_data(lutf_entry_t *list, unsigned int n) {
    lutf_entry_t *entry;
    entry = list_nth_entry(list, n);
    if (entry == NULL) {
        return NULL;
    }
    return entry->data;
}

static inline unsigned int list_length(lutf_entry_t *list) {
    lutf_entry_t *entry;
    unsigned int  length;
    length = 0;
    entry  = list;
    while (entry != NULL) {
        ++length;
        entry = entry->next;
    }
    return length;
}

static inline int list_remove_entry(lutf_entry_t **list, lutf_entry_t *entry) {
    if (list == NULL || *list == NULL || entry == NULL) {
        return 0;
    }
    if (entry->prev == NULL) {
        *list = entry->next;
        if (entry->next != NULL) {
            entry->next->prev = NULL;
        }
    }
    else {
        entry->prev->next = entry->next;
        if (entry->next != NULL) {
            entry->next->prev = entry->prev;
        }
    }
    free(entry);
    return 0;
}

#define SIGUNBLOCK()                                                           \
    do {                                                                       \
        assert(sigprocmask(SIG_UNBLOCK, &sig_act.sa_mask, NULL) == 0);         \
    } while (0)

#define SIGBLOCK()                                                             \
    do {                                                                       \
        assert(sigprocmask(SIG_BLOCK, &sig_act.sa_mask, NULL) == 0);           \
    } while (0)

static int wait_(void) {
    // 等待等待队列中的线程完成
    int flag = 1;
    for (size_t i = 0; i < list_length(env.curr_thread->wait); i++) {
        if (list_nth_data(env.curr_thread->wait, i)->status == lutf_EXIT) {
            continue;
        }
        else {
            flag = 0;
            break;
        }
    }
    if (flag == 1) {
        env.curr_thread->status = lutf_RUNNING;
    }
    return 0;
}

static inline int set_itimer(lutf_prior_t p) {
    // 根据优先级调整运行时间
    switch (p) {
        case NONE: {
            break;
        }
        case LOW: {
            assert(setitimer(ITIMER_VIRTUAL, itimer[LOW], NULL) == 0);
            break;
        }
        case MID: {
            assert(setitimer(ITIMER_VIRTUAL, itimer[MID], NULL) == 0);
            break;
        }
        case HIGH: {
            assert(setitimer(ITIMER_VIRTUAL, itimer[HIGH], NULL) == 0);
            break;
        }
    }
    return 0;
}

// 对周期处理的操作计数
static size_t count = 0;

static void sched(int signo __attribute__((unused))) {
    count++;
    if (sigsetjmp(env.curr_thread->context, SIGVTALRM) == 0) {
        do {
            // 切换到下个线程
            env.curr_thread = env.curr_thread->next;
            // 根据状态
            switch (env.curr_thread->status) {
                // 跳过
                case lutf_READY: {
                    break;
                }
                case lutf_RUNNING: {
                    break;
                }
                case lutf_WAIT: {
                    wait_();
                    break;
                }
                case lutf_SLEEP: {
                    if (clock() > env.curr_thread->resume_time) {
                        env.curr_thread->status = lutf_RUNNING;
                    }
                    break;
                }
                case lutf_SEM: {
                    break;
                }
                case lutf_EXIT: {
                    free(env.curr_thread->stack);
                    env.curr_thread->stack = NULL;
                    list_free(env.curr_thread->wait);
                    env.curr_thread->wait = NULL;
                    break;
                }
            }
        } while (env.curr_thread->status != lutf_RUNNING);
        // TODO:
        // 将退出状态的线程从链表中删除，同时标记等待其完成的线程
        // 这一操作不能太频繁
        if (count % 1000 == 0) {
            ;
        }
        if (env.curr_thread->method == TIME) {
            assert(setitimer(ITIMER_VIRTUAL, itimer[env.curr_thread->prior],
                             NULL) == 0);
        }
        siglongjmp(env.curr_thread->context, 1);
    }
    return;
}

// 构造函数，在 main 之前执行
__attribute__((constructor)) static int init(void) {
    // 初始化 main 线程信息
    lutf_thread_t *thread_main = (lutf_thread_t *)malloc(sizeof(lutf_thread_t));
    assert(thread_main != NULL);
    // 线程 id 为 0
    thread_main->id = 0;
    // 状态为正在运行
    thread_main->status = lutf_RUNNING;
    thread_main->stack  = NULL;
    // 函数指针为 NULL
    thread_main->func = NULL;
    // 参数设为 NULL
    thread_main->arg = NULL;
    // 返回值默认为空
    thread_main->exit_value = NULL;
    // 初始化链表
    thread_main->prev = thread_main;
    thread_main->next = thread_main;
    thread_main->wait = NULL;
    // 优先级默认高
    thread_main->prior       = HIGH;
    thread_main->method      = TIME;
    thread_main->resume_time = 0;
    // 更新全局信息
    env.nid         = 1;
    env.main_thread = thread_main;
    env.curr_thread = thread_main;
    // 默认调度方式
    sig_act.sa_handler = sched;
    sig_act.sa_flags   = 0;
    sigemptyset(&sig_act.sa_mask);
    sigaddset(&sig_act.sa_mask, SIGVTALRM);
    // 注册信号捕捉函数。
    sigaction(SIGVTALRM, &sig_act, NULL);
    assert(setitimer(ITIMER_VIRTUAL, itimer[HIGH], NULL) == 0);
    return 0;
}

// 析构函数，在 main 结束后执行
// 保证所有线程都被回收
__attribute__((destructor)) static int finit(void) {
    // 还原使用的资源
    // 取消时钟
    SIGBLOCK();
    // 恢复之前的系统默认信号和默认信号处理。
    // 将 SIGVTALRM 重置为默认
    sig_act.sa_handler = SIG_DFL;
    sig_act.sa_flags   = SA_RESETHAND;
    sigaction(SIGVTALRM, &sig_act, 0);
    // 如果还有线程没有退出
    lutf_thread_t *p = env.main_thread->next;
    while (p != env.main_thread) {
        if (p->status != lutf_EXIT) {
            // 直接回收
            p->exit_value = NULL;
            p->status     = lutf_EXIT;
            free(p->stack);
            list_free(p->wait);
        }
        p = p->next;
    }
    env.main_thread->status = lutf_EXIT;
    // 释放等待队列
    list_free(env.main_thread->wait);
    // 释放 main 信息占用空间
    free(env.main_thread);
    return 0;
}

int lutf_set_prior(lutf_thread_t *thread, lutf_prior_t p) {
    thread->prior = p;
    return 0;
}

int lutf_create(lutf_thread_t *thread, lutf_fun_t fun, void *arg) {
    assert(thread != NULL);
    assert(fun != NULL);
    // id 默认为 -1
    thread->id = -1;
    // 设置为 READY
    thread->status = lutf_READY;
    thread->stack  = (char *)malloc(LUTF_STACK_SIZE);
    assert(thread->stack != NULL);
    // 要执行的函数指针
    thread->func = fun;
    // 参数
    thread->arg = arg;
    // 退出值
    thread->exit_value  = NULL;
    thread->prev        = thread;
    thread->next        = thread;
    thread->wait        = NULL;
    thread->prior       = MID;
    thread->resume_time = 0;
    return 0;
}

static int add_list(lutf_thread_t *thread) {
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
    return 0;
}

static int run(lutf_thread_t *thread, void **ret) {
    assert(thread != NULL);
    // 添加到线程管理结构
    add_list(thread);
    // 初始化 thread 的上下文
    // 如果不是从 thread 返回，即还没有运行
    if (sigsetjmp(thread->context, SIGVTALRM) == 0) {
        // 将状态更改为 RUNNING
        thread->status = lutf_RUNNING;
        // 等待执行
        if (sigsetjmp(env.curr_thread->context, SIGVTALRM) == 0) {
            // 将 thread 设为当前线程
            env.curr_thread = thread;
            siglongjmp(thread->context, 1);
        }
    }
    // 如果 setjmp 返回值不为 0，说明是从 thread 返回，
    // 这时 env->curr_thread 指向新的线程
    else {
#if defined(__i386__)
        __asm__("mov %0, %%esp"
                :
                : "r"(env.curr_thread->stack + LUTF_STACK_SIZE)
                : "esp");
#elif defined(__x86_64__)
        __asm__("mov %0, %%rsp"
                :
                : "r"(env.curr_thread->stack + LUTF_STACK_SIZE)
                : "rsp");
#elif defined(__arm__) | defined(__aarch64__)
        __asm__("mov sp, %[stack]"
                :
                : [stack] "r"((env.curr_thread->stack + LUTF_STACK_SIZE))
                : "sp");
#endif
        // 执行函数
        env.curr_thread->func(env.curr_thread->arg);
        if (ret != NULL) {
            *ret = env.curr_thread->exit_value;
        }
        env.curr_thread->status = lutf_EXIT;
        if (sigismember(&sig_act.sa_mask, SIGVTALRM) == 1) {
            sched(SIGVTALRM);
        }
        else {
            raise(SIGVTALRM);
        }
    }
    return 0;
}

int lutf_join(lutf_thread_t *thread, void **ret) {
    assert(thread != NULL);
    thread->method = FIFO;
    sigprocmask(SIG_BLOCK, &sig_act.sa_mask, NULL);
    run(thread, ret);
    sigprocmask(SIG_UNBLOCK, &sig_act.sa_mask, NULL);
    return 0;
}

int lutf_detach(lutf_thread_t *thread) {
    assert(thread != NULL);
    thread->method = TIME;
    return run(thread, NULL);
}

int lutf_wait(lutf_thread_t *threads, size_t size) {
    SIGBLOCK();
    env.curr_thread->status = lutf_WAIT;
    for (size_t i = 0; i < size; i++) {
        // 将新进程添加到当前进程的等待链表
        list_append(&env.curr_thread->wait, &threads[i]);
    }
    SIGUNBLOCK();
    raise(SIGVTALRM);
    return 0;
}

int lutf_exit(void *value) {
    SIGBLOCK();
    assert(env.curr_thread != env.main_thread);
    env.curr_thread->exit_value = value;
    env.curr_thread->status     = lutf_EXIT;
    if (env.curr_thread->method == TIME) {
        SIGUNBLOCK();
        raise(SIGVTALRM);
    }
    return 0;
}

int lutf_sleep(lutf_thread_t *thread, size_t sec) {
    thread->status      = lutf_SLEEP;
    thread->resume_time = clock() + sec * CLOCKS_PER_SEC;
    raise(SIGVTALRM);
    return 0;
}

lutf_thread_t *lutf_self(void) {
    return env.curr_thread;
}

int lutf_equal(lutf_thread_t *thread1, lutf_thread_t *thread2) {
    if (thread1->id != thread2->id) {
        return 0;
    }
    return 1;
}

int lutf_cancel(lutf_thread_t *thread) {
    lutf_exit(NULL);
    raise(SIGVTALRM);
    return 0;
}

lutf_S_t *lutf_createS(int ss) {
    lutf_S_t *s = malloc(sizeof(lutf_S_t));
    assert(s != NULL);
    s->s     = ss;
    s->size  = SEM_SIZE;
    s->queue = malloc(sizeof(lutf_thread_t *) * s->size);
    return s;
}

int lutf_P(lutf_S_t *s) {
    if (s->s > 0) {
        s->s -= 1;
        SIGBLOCK();
    }
    else {
        s->s -= 1;
        if (s->size < labs(s->s)) {
            s->size += SEM_SIZE;
            s->queue = realloc(s->queue, sizeof(lutf_thread_t *) * s->size);
        }
        env.curr_thread->status  = lutf_SEM;
        s->queue[labs(s->s) - 1] = env.curr_thread;
        SIGUNBLOCK();
        raise(SIGVTALRM);
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
    SIGBLOCK();
    return 0;
}

#ifdef __cplusplus
}
#endif

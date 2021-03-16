
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

// 全局状态
static lutf_env_t env = {
    .nid         = 0,
    .main_thread = NULL,
    .curr_thread = NULL,
};

// 取消所有定时
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

// 信号处理
struct sigaction sig_act;

// 释放 list
// list: 要释放的 list
static int list_free(lutf_entry_t *list) {
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

// 在 list 头部插入
// list: 要插入的链表
// data: 要插入的数据
// 返回值：成功返回新链表项，失败返回 NULL
static lutf_entry_t *list_prepend(lutf_entry_t **list, lutf_thread_t *data) {
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

// 在 list 尾部插入
// list: 要插入的链表
// data: 要插入的数据
// 返回值：成功返回新链表项，失败返回 NULL
static lutf_entry_t *list_append(lutf_entry_t **list, lutf_thread_t *data) {
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

// 返回前一项
// listentry: 要处理的链表项
// 返回值：成功返回前一项，失败返回 NULL
static lutf_entry_t *list_prev(lutf_entry_t *listentry) {
    if (listentry == NULL) {
        return NULL;
    }
    return listentry->prev;
}

// 返回下一项
// listentry: 要处理的链表项
// 返回值：成功返回下一项，失败返回 NULL
static lutf_entry_t *list_next(lutf_entry_t *listentry) {
    if (listentry == NULL) {
        return NULL;
    }
    return listentry->next;
}

// 返回数据
// listentry: 要处理的链表项
// 返回值：成功返回数据，失败返回 NULL
static lutf_thread_t *list_data(lutf_entry_t *listentry) {
    if (listentry == NULL) {
        return NULL;
    }
    return listentry->data;
}

// 链表第 n 项
// list: 要处理的链表
// n: 第 n 项
// 返回值：成功返回链表项，失败返回 NULL
static lutf_entry_t *list_nth_entry(lutf_entry_t *list, unsigned int n) {
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

// 链表第 n 项数据
// list: 要处理的链表
// n: 第 n 项
// 返回值：成功返回链表项数据，失败返回 NULL
static lutf_thread_t *list_nth_data(lutf_entry_t *list, unsigned int n) {
    lutf_entry_t *entry;
    entry = list_nth_entry(list, n);
    if (entry == NULL) {
        return NULL;
    }
    return entry->data;
}

// 返回链表长度
// list: 要处理的链表
// 返回值：链表长度
static unsigned int list_length(lutf_entry_t *list) {
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

// 删除链表项
// list: 要处理的链表
// entry: 要删除的项
// 返回值：成功返回 0
static int list_remove_entry(lutf_entry_t **list, lutf_entry_t *entry) {
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

// 锁，暂时关闭时钟
#define TICK(x)                                                                \
    do {                                                                       \
        assert(setitimer(ITIMER_VIRTUAL, x, NULL) == 0);                       \
    } while (0)

#define UNTICK()                                                               \
    do {                                                                       \
        assert(setitimer(ITIMER_VIRTUAL, &tick_cancel, NULL) == 0);            \
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

// 对周期处理的操作计数
static size_t count = 0;
// 时钟信号处理
static void sig_alarm_handler(int signo __attribute__((unused))) {
    count++;
    if (setjmp(env.curr_thread->context) == 0) {
        do {
            // 切换到下个线程
            env.curr_thread = env.curr_thread->next;
            // 根据状态
            switch (env.curr_thread->status) {
                // 跳过
                case lutf_READY: {
                    printf("READY: %d\n", env.curr_thread->id);
                    break;
                }
                case lutf_RUNNING: {
                    printf("RUNNING: %d\n", env.curr_thread->id);
                    break;
                }
                case lutf_WAIT: {
                    printf("RUNNING: %d\n", env.curr_thread->id);
                    wait_();
                    break;
                }
                case lutf_SLEEP: {
                    printf("SLEEP\n");
                    if (clock() > env.curr_thread->resume_time) {
                        env.curr_thread->status = lutf_RUNNING;
                    }
                    break;
                }
                case lutf_SEM: {
                    printf("SEM\n");
                    break;
                }
                case lutf_EXIT: {
                    printf("EXIT: %d\n", env.curr_thread->id);
                    // TODO:
                    // 将退出状态的线程从链表中删除，同时标记等待其完成的线程
                    // 这一操作不能太频繁
                    if (count % 1000 == 0) {
                        ;
                    }
                    break;
                }
            }
            // 循环直到 RUNNING 状态的线程
        } while (env.curr_thread->status != lutf_RUNNING);
        // 根据优先级调整运行时间
        switch (env.curr_thread->prior) {
            case LOW: {
                // 开启 timer
                TICK(&tick_low);
                break;
            }
            case MID: {
                // 开启 timer
                TICK(&tick_mid);
                break;
            }
            case HIGH: {
                // 开启 timer
                TICK(&tick_high);
                break;
            }
        }
        // 获取剩余时间
        // getitimer(ITIMER_VIRTUAL, &left);
        // 开始执行
        // 会返回到 lutf_join 处的 setjmp
        longjmp(env.curr_thread->context, 1);
    }
}

static int fifo_handler(void) {
    count++;
    if (setjmp(env.curr_thread->context) == 0) {
        do {
            // 切换到下个线程
            env.curr_thread = env.curr_thread->next;
            // 根据状态
            switch (env.curr_thread->status) {
                // 跳过
                case lutf_READY: {
                    printf("READY: %d\n", env.curr_thread->id);
                    break;
                }
                case lutf_RUNNING: {
                    printf("RUNNING: %d\n", env.curr_thread->id);
                    break;
                }
                case lutf_WAIT: {
                    printf("WAIT: %d\n", env.curr_thread->id);
                    break;
                }
                case lutf_SLEEP: {
                    printf("SLEEP\n");
                    if (clock() > env.curr_thread->resume_time) {
                        env.curr_thread->status = lutf_RUNNING;
                    }
                    break;
                }
                case lutf_SEM: {
                    printf("SEM\n");
                    break;
                }
                case lutf_EXIT: {
                    printf("EXIT: %d\n", env.curr_thread->id);
                    // TODO:
                    // 将退出状态的线程从链表中删除，同时标记等待其完成的线程
                    // 这一操作不能太频繁
                    if (count % 1000 == 0) {
                        ;
                    }
                    break;
                }
            }
            // 循环直到 RUNNING 状态的线程
        } while (env.curr_thread->status != lutf_RUNNING);
        // 开始执行
        // 会返回到 lutf_join 处的 setjmp
        longjmp(env.curr_thread->context, 1);
    }
    return 0;
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
    thread_main->resume_time = 0;
    // 更新全局信息
    env.nid         = 1;
    env.main_thread = thread_main;
    env.curr_thread = thread_main;
    // 默认调度方式
    env.sched_method = FIFO;
    return 0;
}

// 析构函数，在 main 结束后执行
// 保证所有线程都被回收
__attribute__((destructor)) static int finit(void) {
    // 还原使用的资源
    if (env.sched_method == TIME) {
        printf("finit: TIME\n");
        // 取消时钟
        UNTICK();
        // 恢复之前的系统默认信号和默认信号处理。
        // 将 SIGVTALRM 重置为默认
        sig_act.sa_handler = SIG_DFL;
        sigemptyset(&sig_act.sa_mask);
        sig_act.sa_flags = SA_RESETHAND;
        sigaction(SIGVTALRM, &sig_act, 0);
    }
    // 如果还有线程没有退出
    lutf_thread_t *p = env.main_thread->next;
    while (p != env.main_thread) {
        printf("id: %d, status: %d\n", p->id, p->status);
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

int lutf_set_sched(lutf_sched_t method) {
    // 要设置的与正在用的相同
    if (env.sched_method == method) {
        // 直接返回
        return 0;
    }
    env.sched_method = method;
    // 之前是 TIME，现在换成 FIFO
    if (method == FIFO) {
        // 取消定时器
        UNTICK();
        // 将 SIGVTALRM 重置为默认
        sig_act.sa_handler = SIG_DFL;
        sigemptyset(&sig_act.sa_mask);
        sig_act.sa_flags = SA_RESETHAND;
        sigaction(SIGVTALRM, &sig_act, 0);
    }
    // 之前是 FIFO，现在换成 TIME
    else if (method == TIME) {
        // 注册信号处理函数
        sig_act.sa_handler = sig_alarm_handler;
        sig_act.sa_flags   = 0;
        sigemptyset(&sig_act.sa_mask);
        // 注册信号捕捉函数。
        sigaction(SIGVTALRM, &sig_act, NULL);
        TICK(&tick_mid);
    }
    return 0;
}

int lutf_set_prior(lutf_thread_t *thread, lutf_prior_t p) {
    assert(env.sched_method == TIME);
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

//保证局部变量在longjmp过程中一直保存它的值的方法：把它声明为volatile变量。（适合那些在setjmp执行和longjmp返回之间会改变的变量）。
// 存放在内存中的变量，将具有调用longjmp时的值，而在CPU和浮点寄存器中的变量则恢复为调用setjmp函数时的值。
// 优化编译时，register和auto变量都存放在寄存器中，而volatile变量仍存放在内存。

int lutf_join(lutf_thread_t *thread, void **ret) {
    assert(thread != NULL);
    // 添加到线程管理结构
    add_list(thread);
    // 初始化 thread 的上下文
    // 如果不是从 thread 返回，即还没有运行
    if (setjmp(thread->context) == 0) {
        // 将状态更改为 RUNNING
        thread->status = lutf_RUNNING;
        // 等待执行
        if (setjmp(env.curr_thread->context) == 0) {
            // 将 thread 设为当前线程
            env.curr_thread = thread;
            longjmp(thread->context, 1);
        }
    }
    // 如果 setjmp 返回值不为 0，说明是从 thread 返回，
    // 这时 env->curr_thread 指向新的线程
    else {
// TODO: GCC builtin stack switch
#ifdef __x86_64__
        __asm__("mov %0, %%rsp"
                :
                : "r"(env.curr_thread->stack + LUTF_STACK_SIZE));
#endif
        // 执行函数
        env.curr_thread->func(env.curr_thread->arg);
        if (ret != NULL) {
            *ret = env.curr_thread->exit_value;
        }
        env.curr_thread->status = lutf_EXIT;
        fifo_handler();
    }
    return 0;
}

int lutf_detach(lutf_thread_t *thread, void **ret) {
    assert(thread != NULL);
    // 添加到线程管理结构
    add_list(thread);
    // 初始化 thread 的上下文
    // 如果不是从 thread 返回，即还没有运行
    if (setjmp(thread->context) == 0) {
        // 将状态更改为 RUNNING
        thread->status = lutf_RUNNING;
        // 等待执行
        if (setjmp(env.curr_thread->context) == 0) {
            // 将 thread 设为当前线程
            env.curr_thread = thread;
            longjmp(thread->context, 1);
        }
    }
    // 如果 setjmp 返回值不为 0，说明是从 thread 返回，
    // 这时 env->curr_thread 指向新的线程
    else {
// TODO: GCC builtin stack switch
#ifdef __x86_64__
        __asm__("mov %0, %%rsp"
                :
                : "r"(env.curr_thread->stack + LUTF_STACK_SIZE));
#endif
        // 执行函数
        printf("&ret-1: %p\n", &ret);
        printf("ret-1: %p\n", ret);
        env.curr_thread->func(env.curr_thread->arg);
        // BUG: ret 的地址变成 0x03
        printf("&ret: %p\n", &ret);
        printf("ret: %p\n", ret);
        if (ret != NULL) {
            printf("&ret: %p\n", &ret);
            printf("ret: %p\n", ret);
            printf("*ret: %d\n", *ret);
            *ret = env.curr_thread->exit_value;
        }
        env.curr_thread->status = lutf_EXIT;
        raise(SIGVTALRM);
    }
    return 0;
}

int lutf_wait(lutf_thread_t *thread, size_t size) {
    UNTICK();
    env.curr_thread->status = lutf_WAIT;
    for (size_t i = 0; i < size; i++) {
        // 将新进程添加到当前进程的等待链表
        list_append(&env.curr_thread->wait, &thread[i]);
    }
    raise(SIGVTALRM);
    return 0;
}

int lutf_exit(void *value) {
    if (env.sched_method == TIME) {
        UNTICK();
    }
    assert(env.curr_thread != env.main_thread);
    env.curr_thread->exit_value = value;
    env.curr_thread->status     = lutf_EXIT;
    free(env.curr_thread->stack);
    list_free(env.curr_thread->wait);
    if (env.sched_method == TIME) {
        raise(SIGVTALRM);
    }
    return 0;
}

int lutf_sleep(lutf_thread_t *thread, size_t sec) {
    assert(env.sched_method == TIME);
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
    if (thread1->wait != thread2->wait) {
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
    assert(env.sched_method == TIME);
    lutf_S_t *s;
    s = malloc(sizeof(lutf_S_t));
    assert(s != NULL);
    s->s     = ss;
    s->size  = SEM_SIZE;
    s->queue = malloc(sizeof(lutf_thread_t *) * s->size);
    return s;
}

int lutf_P(lutf_S_t *s) {
    assert(env.sched_method == TIME);
    if (s->s > 0) {
        s->s -= 1;
        assert(setitimer(ITIMER_VIRTUAL, &tick_cancel, NULL) == 0);
    }
    else {
        s->s -= 1;
        if (s->size < labs(s->s)) {
            s->size += SEM_SIZE;
            s->queue = realloc(s->queue, sizeof(lutf_thread_t *) * s->size);
        }
        env.curr_thread->status  = lutf_SEM;
        s->queue[labs(s->s) - 1] = env.curr_thread;
        raise(SIGVTALRM);
    }
    return 0;
}

int lutf_V(lutf_S_t *s) {
    assert(env.sched_method == TIME);
    if (s->s >= 0) {
        s->s += 1;
    }
    else {
        s->s += 1;
        s->queue[labs(s->s)]->status = lutf_RUNNING;
    }
    assert(setitimer(ITIMER_VIRTUAL, &tick_cancel, NULL) == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif


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
#include "binary-heap.h"

// 优先队列
static BinaryHeap *prio_queue = NULL;
// 定时器
static struct itimerval tick = {
    .it_interval.tv_sec  = 0,
    .it_interval.tv_usec = 0,
    .it_value.tv_sec     = 0,
    .it_value.tv_usec    = 0,
};
// 全局状态
static lutf_env_t env = {
    .nid         = 0,
    .alg         = 0,
    .tasks       = NULL,
    .curr_thread = NULL,
};

// 运行指定线程
// thread: 要运行的线程
static void run(lutf_thread_t *thread) {
    return;
}

// 调度标记
static uint8_t sched_flag = 0;

// 调度
static RETCODE_t sched() {
    if (setjmp(env.curr_thread->context) == 0) {
        // 切换到下个线程
        if (env.alg == PRIO_QUEUE) {
            env.curr_thread = (lutf_thread_t *)binary_heap_pop(prio_queue);
        }
        do {
            // 根据状态
            switch (env.curr_thread->status) {
                // 跳过
                case lutf_RUNNING: {
                    break;
                }
                // 跳过
                case lutf_EXIT: {
                    break;
                }
                case lutf_WAIT: {
                    break;
                }
                case lutf_SLEEP: {
                    break;
                }
            }
            // 循环直到 READY 状态的线程
        } while (env.curr_thread->status != lutf_RUNNING);
        // 开启 timer
        int ret = setitimer(ITIMER_REAL, &tick, NULL);
        if (ret != 0) {
            return TIMER_ERR;
        }
        // long jmp, 1 indicate that it's jumped, not return directly.
        // 开始执行
        // 会返回到 lutf_add_sched 处的 setjmp
        longjmp(env.curr_thread->context, 1);
    }
    return SUCCESS;
}

// 更新任务执行顺序
static void update_tasks();

// 时钟信号处理
static void sig_alarm_handler(int signo) {
    if (signo != SIGALRM) {
        return;
    }
    if (sched_flag == 1) {
        sched();
        printf("entries: %d\n", binary_heap_num_entries(prio_queue));
    }
    return;
}

// 优先级比较
static int prio_compare(void *location1, void *location2) {
    if (((lutf_thread_t *)location1)->prio <
        ((lutf_thread_t *)location2)->prio) {
        return -1;
    }
    else if (((lutf_thread_t *)location1)->prio >
             ((lutf_thread_t *)location2)->prio) {
        return 1;
    }
    else {
        return 0;
    }
}

RETCODE_t lutf_init(SCHED_ALG_t alg) {
    int ret = 0;
    // 注册信号处理函数
    if (signal(SIGALRM, sig_alarm_handler) == SIG_ERR) {
        return ALARM_HANDLER_ERR;
    }
    if (alg == PRIO_QUEUE) {
        // 初始化 main 线程信息
        lutf_thread_t *thread_main;
        // 为结构体分配内存
        thread_main = (lutf_thread_t *)malloc(sizeof(lutf_thread_t));
        if (thread_main == NULL) {
            return NO_ENOUGH_MEM;
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
        thread_main->argv = NULL;
        // 返回值为空
        thread_main->ret = NULL;
        // 返回状态默认为成功
        thread_main->exit_code = 0;
        // 初始化优先队列
        prio_queue = binary_heap_new(BINARY_HEAP_TYPE_MAX, prio_compare);
        if (prio_queue == NULL) {
            return PRIO_QUEUE_NEW_FAILED;
        }
        // 将 main 线程插入
        ret = binary_heap_insert(prio_queue, thread_main);
        if (ret == 0) {
            return PRIO_QUEUE_INSERT_FAILED;
        }
        // 更新全局信息
        env.nid         = 1;
        env.alg         = PRIO_QUEUE;
        env.tasks       = (void *)prio_queue;
        env.curr_thread = thread_main;
        // 设置定时器
        // SLICE 后启动计时器
        tick.it_value.tv_sec  = 0;
        tick.it_value.tv_usec = SLICE;
        // 每隔 SLICE 到期一次
        tick.it_interval.tv_sec  = 0;
        tick.it_interval.tv_usec = SLICE;
        // 开启 timer
        ret = setitimer(ITIMER_REAL, &tick, NULL);
        if (ret != 0) {
            return TIMER_ERR;
        }
    }
    return SUCCESS;
}

lutf_thread_t *lutf_create_task(lutf_fun_t fun, void *argv) {
    lutf_thread_t *thread;
    // 分配线程空间
    thread = (lutf_thread_t *)malloc(sizeof(lutf_thread_t));
    if (thread == NULL) {
        return NULL;
    }
    // 分配线程栈
    thread->stack = (char *)malloc(LUTF_STACK_SIZE);
    if (thread->stack == NULL) {
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
    return thread;
}

RETCODE_t lutf_add_sched(lutf_thread_t *thread) {
    int ret = 0;
    // 添加到线程管理结构
    if (env.alg == PRIO_QUEUE) {
        ret = binary_heap_insert(prio_queue, thread);
        if (ret == 0) {
            return PRIO_QUEUE_INSERT_FAILED;
        }
    }
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
        printf("111\n");
    }
    // 如果 setjmp 返回值不为 0，说明是从 thread 返回，
    // 这时 env->curr_thread 指向新的线程
    else {
        printf("222\n");
        // 手动切换线程栈
#ifdef __x86_64__
        __asm__("mov %0, %%rsp"
                :
                : "r"(env.curr_thread->stack + LUTF_STACK_SIZE));
#endif
        // 执行函数
        env.curr_thread->func(env.curr_thread->argv);
        env.curr_thread->status = lutf_EXIT;
        ret                     = setitimer(ITIMER_REAL, &tick, NULL);
        printf("333\n");
        if (ret != 0) {
            return TIMER_ERR;
        }
        printf("444\n");
        raise(SIGALRM);
        printf("555\n");
    }
    printf("666\n");
    return SUCCESS;
}

RETCODE_t lutf_del_task(lutf_thread_t *thread) {

    return SUCCESS;
}

void lutf_sched_start(void) {
    sched_flag = 1;
    return;
}

void lutf_sched_end(void) {
    sched_flag = 0;
    return;
}

uint8_t lutf_get_sched_status(void) {
    return sched_flag;
}

#ifdef __cplusplus
}
#endif

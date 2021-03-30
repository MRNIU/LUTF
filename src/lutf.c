
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

// TODO: clean

// sched method
typedef enum {
    FIFO = 1,
    TIME = 2,
} lutf_sched_t;

// thread id type
typedef ssize_t lutf_task_id_t;

typedef struct waitt {
    struct lutf_thread *thread;
    struct waitt *      prev;
    struct waitt *      next;
} wait_t;

// thread
typedef struct lutf_thread {
    // thread id
    lutf_task_id_t id;
    // thread status
    lutf_status_t status;
    // thread stack
    char *stack;
    // function
    lutf_fun_t func;
    // function parameter
    void *arg;
    // exit value
    void *exit_value;
    // jmp_buf
    jmp_buf context;
    // prev thread
    struct lutf_thread *prev;
    // next thread
    struct lutf_thread *next;
    // wait list
    wait_t *wait;
    // how many threads are waiting
    size_t wait_count;
    // how many threads are waiting for
    size_t waited;
    // prior
    lutf_prior_t prior;
    // resume time
    clock_t resume_time;
    // sched
    lutf_sched_t method;
} lutf_thread_t;

// global val
typedef struct lutf_env {
    size_t         nid;
    lutf_thread_t *main_thread;
    lutf_thread_t *curr_thread;
} lutf_env_t;

static lutf_env_t env = {
    .nid         = 0,
    .main_thread = NULL,
    .curr_thread = NULL,
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

#define SIGUNBLOCK()                                                           \
    do {                                                                       \
        assert(sigprocmask(SIG_UNBLOCK, &sig_act.sa_mask, NULL) == 0);         \
    } while (0)

#define SIGBLOCK()                                                             \
    do {                                                                       \
        assert(sigprocmask(SIG_BLOCK, &sig_act.sa_mask, NULL) == 0);           \
    } while (0)

static int wait_(void) {
    wait_t *tmp  = env.curr_thread->wait->next;
    wait_t *tmp2 = tmp->prev;
    while (tmp != env.curr_thread->wait) {
        if (tmp->thread->status == lutf_EXIT) {
            tmp->thread->waited--;
            env.curr_thread->wait_count--;
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
            free(tmp);
            tmp = tmp2;
        }
        if (env.curr_thread->wait_count == 1) {
            env.curr_thread->status = lutf_RUNNING;
            break;
        }
        tmp = tmp->next;
    }
    return 0;
}

// TODO: if main is run continuously, mask SIGVTALRM
// TODO: pref cycle
static void sched(int signo __attribute__((unused))) {
    if (sigsetjmp(env.curr_thread->context, SIGVTALRM) == 0) {
        do {
            env.curr_thread = env.curr_thread->next;
            switch (env.curr_thread->status) {
                case lutf_READY: {
                    break;
                }
                case lutf_RUNNING: {
                    break;
                }
                case lutf_EXIT: {
                    free(env.curr_thread->stack);
                    env.curr_thread->stack = NULL;
                    if (env.curr_thread->waited == 0) {
                        // remove from list
                        env.curr_thread->prev->next = env.curr_thread->next;
                        env.curr_thread->next->prev = env.curr_thread->prev;
                        lutf_thread_t *tmp          = env.curr_thread->prev;
                        free(env.curr_thread);
                        env.curr_thread = tmp;
                    }
                    break;
                }
                case lutf_WAIT: {
                    wait_();
                    break;
                }
                case lutf_SEM: {
                    break;
                }
                case lutf_SLEEP: {
                    if (clock() > env.curr_thread->resume_time) {
                        env.curr_thread->status = lutf_RUNNING;
                    }
                    break;
                }
            }
        } while (env.curr_thread->status != lutf_RUNNING);
        if (env.curr_thread->method == TIME) {
            assert(setitimer(ITIMER_VIRTUAL, itimer[env.curr_thread->prior],
                             NULL) == 0);
        }
        siglongjmp(env.curr_thread->context, 1);
    }
    return;
}

__attribute__((constructor)) static int init(void) {
    lutf_thread_t *thread_main = (lutf_thread_t *)malloc(sizeof(lutf_thread_t));
    assert(thread_main != NULL);
    thread_main->id           = 0;
    thread_main->status       = lutf_RUNNING;
    thread_main->stack        = NULL;
    thread_main->func         = NULL;
    thread_main->arg          = NULL;
    thread_main->exit_value   = NULL;
    thread_main->prev         = thread_main;
    thread_main->next         = thread_main;
    thread_main->wait         = (wait_t *)malloc(sizeof(wait_t));
    thread_main->wait->thread = thread_main;
    thread_main->wait->prev   = thread_main->wait;
    thread_main->wait->next   = thread_main->wait;
    thread_main->wait_count   = 1;
    thread_main->waited       = 0;
    thread_main->prior        = HIGH;
    thread_main->method       = TIME;
    thread_main->resume_time  = 0;
    env.nid                   = 1;
    env.main_thread           = thread_main;
    env.curr_thread           = thread_main;
    sig_act.sa_handler        = sched;
    sig_act.sa_flags          = 0;
    sigemptyset(&sig_act.sa_mask);
    sigaddset(&sig_act.sa_mask, SIGVTALRM);
    sigaction(SIGVTALRM, &sig_act, NULL);
    assert(setitimer(ITIMER_VIRTUAL, itimer[HIGH], NULL) == 0);
    return 0;
}

// TODO: wait GC
__attribute__((destructor)) static int finit(void) {
    SIGBLOCK();
    sig_act.sa_handler = SIG_DFL;
    sig_act.sa_flags   = SA_RESETHAND;
    sigdelset(&sig_act.sa_mask, SIGVTALRM);
    sigaction(SIGVTALRM, &sig_act, 0);
    lutf_thread_t *tmp = env.main_thread->next;
    while (tmp != env.main_thread) {
        if (tmp->status != lutf_EXIT) {
            free(tmp->stack);
            free(tmp->wait);
            free(tmp);
        }
        tmp = tmp->next;
    }
    env.main_thread->status = lutf_EXIT;
    free(env.main_thread->wait);
    free(env.main_thread);
    return 0;
}

int lutf_set_prior(lutf_t *t, lutf_prior_t p) {
    lutf_thread_t *thread = *t;
    thread->prior         = p;
    return 0;
}

int lutf_create(lutf_t *t, lutf_fun_t fun, void *arg) {
    assert(t != NULL);
    assert(fun != NULL);
    *t                    = malloc(sizeof(lutf_thread_t));
    lutf_thread_t *thread = *t;
    thread->id            = -1;
    thread->status        = lutf_READY;
    thread->stack         = (char *)malloc(LUTF_STACK_SIZE);
    assert(thread->stack != NULL);
    thread->func         = fun;
    thread->arg          = arg;
    thread->exit_value   = NULL;
    thread->prev         = thread;
    thread->next         = thread;
    thread->wait         = (wait_t *)malloc(sizeof(wait_t));
    thread->wait->thread = thread;
    thread->wait->prev   = thread->wait;
    thread->wait->next   = thread->wait;
    thread->wait_count   = 1;
    thread->waited       = 0;
    thread->prior        = MID;
    thread->resume_time  = 0;
    return 0;
}

static int add_list(lutf_thread_t *thread) {
    lutf_thread_t *prev      = env.main_thread->prev;
    lutf_thread_t *next      = env.main_thread;
    lutf_thread_t *new_entry = thread;
    prev->next               = new_entry;
    next->prev               = new_entry;
    new_entry->prev          = prev;
    new_entry->next          = next;
    thread->id               = env.nid;
    env.nid += 1;
    return 0;
}

static int add_wait_list(lutf_thread_t *thread) {
    wait_t *prev      = env.curr_thread->wait->prev;
    wait_t *next      = env.curr_thread->wait;
    wait_t *new_entry = (wait_t *)malloc(sizeof(wait_t));
    new_entry->prev   = new_entry;
    new_entry->next   = new_entry;
    new_entry->thread = thread;
    prev->next        = new_entry;
    next->prev        = new_entry;
    new_entry->prev   = prev;
    new_entry->next   = next;
    env.curr_thread->wait_count++;
    return 0;
}

static int run(lutf_thread_t *thread) {
    assert(thread != NULL);
    add_list(thread);
    if (sigsetjmp(thread->context, SIGVTALRM) == 0) {
        thread->status = lutf_RUNNING;
        if (sigsetjmp(env.curr_thread->context, SIGVTALRM) == 0) {
            env.curr_thread = thread;
            siglongjmp(thread->context, 1);
        }
    }
    else {
        // TODO: stack direction processing
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
        env.curr_thread->func(env.curr_thread->arg);
        env.curr_thread->status = lutf_EXIT;
        sched(SIGVTALRM);
    }
    return 0;
}

int lutf_join(lutf_t *t, void **ret) {
    assert(t != NULL);
    SIGBLOCK();
    lutf_thread_t *thread = *t;
    thread->method        = FIFO;
    run(thread);
    if (ret != NULL) {
        *ret = thread->exit_value;
    }
    return 0;
}

int lutf_detach(lutf_t *t) {
    assert(t != NULL);
    SIGUNBLOCK();
    lutf_thread_t *thread = *t;
    thread->method        = TIME;
    run(thread);
    return 0;
}

int lutf_wait(lutf_t *t, size_t size) {
    assert(t != NULL);
    assert(size != 0);
    SIGBLOCK();
    env.curr_thread->status = lutf_WAIT;
    for (size_t i = 0; i < size; i++) {
        add_wait_list((lutf_thread_t *)t[i]);
        ((lutf_thread_t *)t[i])->waited++;
    }
    sched(SIGVTALRM);
    return 0;
}

int lutf_exit(void *value) {
    SIGBLOCK();
    if (env.curr_thread == env.main_thread) {
        exit(0);
    }
    else {
        env.curr_thread->exit_value = value;
        env.curr_thread->status     = lutf_EXIT;
        if (env.curr_thread->method == TIME) {
            SIGUNBLOCK();
            raise(SIGVTALRM);
        }
        else {
            sched(SIGVTALRM);
        }
    }
    return 0;
}

int lutf_sleep(lutf_t *t, size_t sec) {
    SIGBLOCK();
    lutf_thread_t *thread = *t;
    thread->status        = lutf_SLEEP;
    thread->resume_time   = clock() + sec * CLOCKS_PER_SEC;
    SIGUNBLOCK();
    raise(SIGVTALRM);
    return 0;
}

lutf_t lutf_self(void) {
    return (lutf_t)env.curr_thread;
}

int lutf_equal(lutf_t *t1, lutf_t *t2) {
    assert(t1 != NULL);
    assert(t2 != NULL);
    lutf_thread_t *thread1 = *t1;
    lutf_thread_t *thread2 = *t2;
    return (thread1->id == thread2->id) ? 1 : 0;
}

// TODO
lutf_status_t lutf_status(lutf_t *t) {
    assert(t != NULL);
    lutf_thread_t *thread = *t;
    if (thread == NULL) {
        return lutf_EXIT;
    }
    else {
        return thread->status;
    }
}

int lutf_cancel(lutf_t *t) {
    assert(t != NULL);
    SIGBLOCK();
    lutf_thread_t *thread = *t;
    if (thread != env.main_thread) {
        thread->exit_value = NULL;
        thread->status     = lutf_EXIT;
        sched(SIGVTALRM);
    }
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
        SIGUNBLOCK();
    }
    else {
        s->s -= 1;
        if (s->size < labs(s->s)) {
            s->size += SEM_SIZE;
            s->queue = realloc(s->queue, sizeof(lutf_thread_t *) * s->size);
        }
        env.curr_thread->status  = lutf_SEM;
        s->queue[labs(s->s) - 1] = (lutf_t *)env.curr_thread;
        raise(SIGVTALRM);
    }
    return 0;
}

int lutf_V(lutf_S_t *s) {
    SIGBLOCK();
    if (s->s >= 0) {
        s->s += 1;
    }
    else {
        s->s += 1;
        ((lutf_thread_t *)s->queue[labs(s->s)])->status = lutf_RUNNING;
    }
    SIGUNBLOCK();
    return 0;
}

#ifdef __cplusplus
}
#endif

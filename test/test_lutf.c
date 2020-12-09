
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// lutf.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "lutf.h"

static uint64_t i = 0;

void *fun1(void *argv) {
    unsigned long long a = ((unsigned long long *)argv)[0];
    printf("%llu\n", a);
    if (a == 2000000) {
        ;
    }
    return NULL;
}

void *fun2(void *argv) {
    uint64_t j = 0;
    j          = i * i * i / 2 + 34;
    i++;
    if (i == 2000000) {
        printf("end\n");
    }
    return NULL;
}

int main(int argc __unused, char **argv __unused) {
    RETCODE_t ret = SUCCESS;
    ret           = lutf_init(PRIO_QUEUE);
    if (ret != SUCCESS) {
        printf("init error\n");
    }
    lutf_thread_t *t;
    for (int i = 0; i < 200; i++) {
        t   = lutf_create_task(fun2, NULL);
        ret = lutf_add_sched(t);
        if (ret != SUCCESS) {
            printf("Add sched error\n");
        }
    }
    // lutf_sched_start();

    printf("End.");
    return 0;
}

#ifdef __cplusplus
}
#endif

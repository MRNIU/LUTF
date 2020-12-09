
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// lutf.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"

static uint64_t i = 0;

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
    for (int i = 0; i < 2000000; i++) {
        fun2(NULL);
    }

    printf("End.");
    return 0;
}

#ifdef __cplusplus
}
#endif


// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// test_manage.c for MRNIU/LUTF.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "lutf.h"

static int test1(void *argv __attribute__((unused))) {
    return 0;
}

// 创建与销毁
// 动态加入
// 动态销毁
// 等待
// 同步互斥
static int basic(void) {
    return 0;
}

// 进程管理测试
int test_manage(void) {
    printf("--------manage--------\n");
    assert(basic() == 0);
    return 0;
}

#ifdef __cplusplus
}
#endif

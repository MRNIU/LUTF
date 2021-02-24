
// This file is a part of MRNIU/LUTF
// (https://github.com/MRNIU/LUTF).
//
// test.h for MRNIU/LUTF.

#ifndef _TEST_H_
#define _TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*UnitTestFunction)(void);

void run_tests(UnitTestFunction *tests);

// 任务抽象
int test_task(void);
// 任务管理
int test_manage(void);
// 任务调度
int test_sched(void);
// 任务 IO
int test_io(void);
// FIFO
int test_fifo(void);

#ifdef __cplusplus
}
#endif

#endif /* _TEST_H_ */

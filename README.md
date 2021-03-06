# LUTF

Linux 用户级任务调度框架。

关键词：协程，跨平台，用户级，setjmp，signal，C



|              | OSX/x86_64 | OSX/ARM | LINUX/x86_64 | LINUX/ARM |
| :----------: | :--------: | :-----: | :----------: | :-------: |
|  GCC 7.3.0   |     /      |         |              |     ✅     |
|  GCC 7.5.0   |     ✅      |         |              |           |
|  GCC 9.3.0   |     /      |         |      ✅       |           |
|  GCC 10.2.0  |     ✅      |         |              |           |
| Clang 12.0.0 |     ✅      |         |              |           |

## 接口

|                             接口                             |                          参数                          |                返回值                 |           功能           |
| :----------------------------------------------------------: | :----------------------------------------------------: | :-----------------------------------: | :----------------------: |
|  int lutf_set_prior(lutf_thread_t *thread, lutf_prior_t p)   |             thread: 要设置的线程;p: 优先级             |              成功返回 0               |        设置优先级        |
| int lutf_create(lutf_thread_t *thread, lutf_fun_t fun, void *arg) | thread: 线程结构体; fun: 要执行的函数; arg: fun 的参数 |              成功返回 0               |         创建线程         |
|       int lutf_join(lutf_thread_t *thread, void **ret)       |            thread: 线程结构体; ret: 返回值             |              成功返回 0               |     等待 thread 结束     |
|            int lutf_detach(lutf_thread_t *thread)            |                  thread: 线程结构体;                   |              成功返回 0               |     并发执行 thread      |
|      int lutf_wait(lutf_thread_t *threads, size_t size)      |            threads: 线程结构体; size: 数量             |              成功返回 0               |     等待 thread 结束     |
|                  int lutf_exit(void *value)                  |                     value: 退出值                      |              成功返回 0               |         退出线程         |
|      int lutf_sleep(lutf_thread_t *thread, size_t sec)       |                  thread: 线程结构体;                   |              成功返回 0               |           睡眠           |
|                lutf_thread_t *lutf_self(void)                |                           /                            | 成功返回 thread 结构体，失败返回 NULL |     获取当前线程 id      |
| int lutf_equal(lutf_thread_t *thread1, lutf_thread_t *thread2) |       thread1: 线程结构体1;thread2: 线程结构体2        |        相等返回 1，否则返回 0         | 比较两个 thread 是否相同 |
|            int lutf_cancel(lutf_thread_t *thread)            |                   thread: 线程结构体                   |              成功返回 0               |       取消 thread        |
|          lutf_status_t lutf_status(lutf_t *thread);          |                   thread: 线程结构体                   |               线程状态                |       返回线程状态       |
|                lutf_S_t *lutf_createS(int ss)                |                     ss: 信号量数量                     |  成功返回信号量结构体，失败返回 NULL  |        创建信号量        |
|                   int lutf_P(lutf_S_t *s)                    |                    s: 信号量结构体                     |              成功返回 0               |      信号量 P 操作       |
|                   int lutf_V(lutf_S_t *s)                    |                    s: 信号量结构体                     |              成功返回 0               |      信号量 V 操作       |



##  使用方法

添加 `luth.h` 到你的代码中，并将编译出的库文件加入链接

```shell
# 示例
# 编译
git clone https://github.com/MRNIU/LUTF.git
cd LUTF/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make

# 使用
# 将 LUTF/include/lutf.h 与 LUTF/build/lib/liblutf.dylib 复制到你的代码目录
# 添加库搜索路径
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
# 编译
gcc src.c -llutf -L.
```

- 示例

    ```c
    # src.c

    #include "stdio.h"
    #include "stdlib.h"
    #include "assert.h"
    #include "lutf.h"

    void *fun1(void *arg) {
        if (arg != NULL) {
            printf("arg1: %s\n", (char *)arg);
        }
        for(size_t i=0;i<100000;i++){
            // Do something
        }
        lutf_exit((void *)"This is fun1 exit value");
        return NULL;
    }

    void *fun2(void *arg) {
        if (arg != NULL) {
            printf("arg2: %s\n", (char *)arg);
        }
        for(size_t i=0;i<100000;i++){
            // Do something
        }
        lutf_exit(NULL);
        return NULL;
    }

    int main(int argc, char**argv){
        lutf_t thread1;
        lutf_t thread2;
        char *  arg1  = "This is fun1's arg";
        char *  arg2  = "This is fun2's arg";
        void ** ret     = malloc(sizeof(char *));
        assert(lutf_create(&thread1, fun1, (void *)arg1) == 0);
        assert(lutf_create(&thread2, fun2, (void *)arg2) == 0);
        lutf_join(&thread1, ret);
        printf("ret: %s\n", (char *)ret[0]);
        lutf_detach(&thread2);
        return 0;
    }
    ```

    ```shell
    # 编译
    gcc src.c -llutf -L.
    ```



## 构建

```shell
git clone https://github.com/MRNIU/LUTF.git
cd LUTF/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=DEBUG ..
make
```



## 性能

TODO

测试代码

```c
#include "lutf.h"
#include "math.h"
int main(int argc, char ** argv) {
    return 0;
}
```

编译命令

```shell
git clone https://github.com/MRNIU/LUTF.git
cd LUTF/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=DEBUG ..
make
```


基于 MacBook Pro (15-inch, 2017)  16G 测试

| 运行方式 | 运行用时 | 内存占用 |      |      |
| :------: | :------: | :------: | ---- | ---- |
|   LUTF   |          |          |      |      |
| pthread  |          |          |      |      |
| 直接调用 |          |          |      |      |
|   fork   |          |          |      |      |



## 参考

[cadmuxe/LWT](https://github.com/cadmuxe/LWT)

[cppreference/setjmp](https://en.cppreference.com/w/cpp/utility/program/setjmp)

[cppreference/signal](https://en.cppreference.com/w/c/program/signal)

[man/setitimer](https://man7.org/linux/man-pages/man2/setitimer.2.html)

[linux进程调度算法:分时调度策略、FIFO调度策略、RR调度策略](https://blog.csdn.net/qq_32811489/article/details/70768264)


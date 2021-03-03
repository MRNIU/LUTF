# LUTF

Linux Userspace Task Framework.

Linux 用户级任务调度框架.

lightweight pthread.

轻量 pthread.

Based on setjmp and signal.

基于 setjmp 与 signal.

cross-platform.

跨平台.



## 功能 Features







## 性能 Performance

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
make gcc-10
```



基于 MacBook Pro (15-inch, 2017)  16G 测试

| 运行方式 | 运行用时 | 内存占用 |      |      |
| :------: | :------: | :------: | ---- | ---- |
|   LUTF   |          |          |      |      |
| pthread  |          |          |      |      |
| 直接调用 |          |          |      |      |
|   fork   |          |          |      |      |



## 原理 Principle




##  使用方法 Usage

你可以通过两种方法使用 LUTF

- 编译

    在编译你的代码时将 LUTF 的代码一同进行编译

- 库

    LUTF 提供 .a 库



## 开发 Dev

LUTF 使用 cmake 构建

```shell
git clone https://github.com/MRNIU/LUTF.git
cd LUTF/
mkdir build
cmake -DCMAKE_BUILD_TYPE=DEBUG ..
make
```



## 参考 Refs

[cadmuxe/LWT](https://github.com/cadmuxe/LWT)

[cppreference/setjmp](https://en.cppreference.com/w/cpp/utility/program/setjmp)

[cppreference/signal](https://en.cppreference.com/w/c/program/signal)

[man/setitimer](https://man7.org/linux/man-pages/man2/setitimer.2.html)

[linux进程调度算法:分时调度策略、FIFO调度策略、RR调度策略](https://blog.csdn.net/qq_32811489/article/details/70768264)

# goc  
[![workflow](https://github.com/jingyugao/tco/workflows/C/C++%20CI/badge.svg)]( https://github.com/jingyugao/tco/workflows/C/C++%20CI/badge.svg)

Goroutine implement of c.

## context
程序运行时的环境为context，即cpu的状态。通过操作context即可实现交替执行代码，实现类似并行的效果。
cpu的状态即其中寄存器的值。由于切换上下文只会通过函数调用发生。因此只需考虑函数调用相关的寄存器。
`rbx(通用), rsp(栈顶), rbp(栈基), r12(通用), r13(通用), r14(通用), r15(通用), pc(代码), rdi(参数)`
上下文切换包括`SaveContext`和`GetContext`两个函数即可实现控制流跳转。

## 协程
协程即用户态进程。每个协程都需要自己的context。除了寄存器外，还需要栈空间，为了简单考虑，
采用非共享栈的方式，每个协程固定1M的栈空间。由于内存分配是lazy-initialization，不会造成太大的资源浪费。 
通过切换context即可实现协程切换。
为了交替执行代码,需要在代码中插入`co_schedule`，进行协程切换。包括：
1. 手动加上能够触发调度的代码。
2. 通过gcc编译选项，在函数调用前插入指令。

## 调度
最简单的调度算法即FIFO。为了效率起见，调度不应该太频繁。`co_schedule`分为*强制*和*建议*两种模式。
在*建议*模式下，应该考虑效率因素来决定是否调度。
通过时间片机制可以让多个协程比较公平且效率的运行。
在执行协程前，记录当前时间。通过后台线程定期扫描，如果时间片到期，设置调度标志。
此时通过`co_schedule`即可触发调度。

## 相关原语
- `timeSleep`:协程挂起自己，触发调度。一段时间后，重新把此协程加入runq.
- `GoSched`:把自己放到runq中，触发调度。
- `semaphore`:信号量，得不到信号量时挂起自己。释放时唤醒一个。






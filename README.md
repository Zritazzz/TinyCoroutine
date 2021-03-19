
TinyCoroutine
=======
C + + coroutine library demo based on Linux

简单的C++协程库demo

* 一个调度器可以拥有多个协程
* 通过`uthread_create`创建一个协程
* 通过`uthread_resume`运行或者恢复运行一个协程
* 通过`uthread_yield`挂起一个协程，并切换到主进程中
* 通过`schedule_finished` 判断调度器中的协程是否全部运行完毕
* 每个协程默认拥有128Kb的栈，采用私有栈设计


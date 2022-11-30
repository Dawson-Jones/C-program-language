# 信号

信号是软件中断

信号的响应依赖于中断



*kill*: send any signal to any process group or process.

`kill -l `查看当前信号

*raise*: sends a signal to the calling process or thread.

*alarm*: arranges for a SIGALRM signal to be delivered to the calling process in seconds seconds.



## [信号会打断阻塞的系统调用](./signal/2_sighandle.c)





## 可重入函数

>  可以在这个函数执行的任何时刻中断它，转入OS调度下去执行另外一段代码
>
> 返回控制时不会出现什么错误

比如返回值在静态区的函数是不可重入的， 因为下一次的调用会覆盖上一次的结果

所有的系统调用都是可重入的

## 信号的响应过程



当进程调度到当前进程时， 内核恢复进程现场，会做 `mask & penind`

mask 和 pending 都是 32 位, mask 代表信号屏蔽字， pending 代表信号

mask 初始每位都是 1 

1. 信号到来
2. 与操作发现信号
3. 将当前进程恢复现场的地址修改为注册的 `sig_handle` 并把 mask 和 pending 的那一位**置为 0**
4. 当执行完 si `g_handle` 后, 将 **mask 恢复**
5. 再做一次与操作, 发现没有信号, **地址恢复**

> 为什么要将 mask 置为 0, 他前后明明都是1, 且置为 0 后, 如果信号屏蔽字来了冲突..
>
> mask 置为 0, 防止重入现象
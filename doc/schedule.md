## get g to execute
1. 隔一段tikc,从globq获取一个
2. 获取p的runq
3. 获取netpoll的
4. steal其他p的。

## p的状态
schedinit中，初始化p的个数为`GOMAXPROC`，
`sched.pidle`为链表，存了所有空闲的p(runqsize为0)
`pidleput`和`pidleput`操作这个链表。
程序的P是固定的，`startm`的时候如果获取不到空闲的P，则会直接返回。




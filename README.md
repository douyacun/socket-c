# C语言socket编程


# client
1. 使用recv() == 0 判断是否与服务器断开连接
2. 使用 select() 监视文件描述符的变化情况


# server
1. 生产者-消费者
2. 线程处理客户端
3. 信号量mutex保护共享变量

TODO::
1. 服务端使用epoll模型
2. 服务端检测客户端断开连接

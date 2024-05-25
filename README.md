# ZeroCamera_demo
ZeroCamera for study
zhoucc2008@outlook.com

## 项目简介
从零开始开始一个linux应用程序开发框架，用于个人学习

项目目标：适用于IPC网络摄像头 --设计音视频采集,编码,传输,显示。
涉及点：涉及linux bsp,驱动开发,应用软件设计,音视频编码,传输,网络通信,视频处理等技术

## 设计思想
### 1.C/C++混合编程
a.考虑到应用于嵌入式设备，BSW基础库提供C接口实现，基础库设计可复用c语言开发项目。

b.使用RAII编程思想，对c接口进行C++二次封装。暂时构造中未采用抛异常方式，采用二段式构造,未使用智能指针做RAII管理，仍采用显示delete方式释放资源

c.C++11编程规范，大部分代码采用C with class基本语法。对程序员学习开发维护成本低，简单实用。
(毕竟回字有4种写法)

### 2.开源模块
1.项目引用nng,spdlog,cjson,openssl等优秀开源项目代码（多用轮子）

2.C++对开源代码(nng/spdlog/cjson库)进行简单二次封装；
剥离上层业务应用与开源代码的依赖，后续有能力者，可以自行实现功能替换开源代码

### 3.moudle通信框架
1.多module设计，暂定zc_sys,zc_codec,zc_rtsp,(后续zc_webs,zc_ui)多个进程。模块解耦

2.多moudle,主进程zc_sys提供注册服务，控制/设置命令直接下发到对应module,状态变更,通过订阅消息分发到不同module.

3.moudle之间面向消息协议编程，只依赖于制订的协议。

4.module之间消息使用nng库req/rep,pub/sub通信模型进程通信

5.其他开发者可以增加进程进行开发对接不同协议

(暂时考虑多进程)分布式机制，注册服务
原因：考虑其他开发者可以增加进程进行开发对接不同协议。而不影响其他进程运行稳定性。

多进程模式:设计进程间通信，模块与模块项目解耦；框架复杂，带来额外的系统资源
单个业务模块崩溃，不影响其他的业务模块，以及整个系统的稳定行

TODO:nng客户端加锁

### 4.跨进程音视频fifo设计
1.跨进程共享buf,如何减少拷贝？
2.借鉴kfifo思想实现环形fifo
3.写入者不判断读指针位置，每个读者自行保留读指针位置
4.创建shmfifo写着时，同时打开一个FIFO管道,配合epoll,用于写入帧数据时，实现触发可读fifo可read事件，实现事件驱动read数据
5.写入帧数据时,先写入zc_frame_t帧头，在通过回调函数追加写入帧数据(可多次调用PutAppending多次追加数据，最后一包设置end标识)

2.待完善

## build
### build for linux x64 gcc/g++
```
./build.sh
```
### build for linux hi3519dv500 gcc/g++
```
./build.sh soc=hi3519dv500
```

##
参考引用:
1.openhisilicon
2.media-server
3.nng
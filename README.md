# ZeroCamera_demo
ZeroCamera for study
zhoucc2008@outlook.com

## 项目简介
从零开始开始一个linux应用程序开发框架，用于个人学习 \
项目目标：适用于IPC网络摄像头 --设计音视频采集,编码,传输,显示。\
涉及点：涉及linux bsp,驱动开发,应用软件设计,音视频编码,传输,网络通信,视频处理等技术


## 设计思想
### 1.C/C++混合编程
a.考虑到应用于嵌入式设备，BSW基础库提供C接口实现，基础库设计可复用c语言开发项目。 \
b.使用RAII编程思想，对c接口进行C++二次封装。暂时构造中未采用抛异常方式，采用二段式构造,未使用智能指针做RAII管理，仍采用显示delete方式释放资源 \
c.C++11编程规范，大部分代码采用C with class基本语法。对程序员学习开发维护成本低，简单实用。
(毕竟回字有4种写法)

### 2.开源模块
1.项目引用nng,spdlog,cjson,openssl等优秀开源项目代码（多用轮子） \
2.C++对开源代码(nng/spdlog/cjson库)进行简单二次封装； \
剥离上层业务应用与开源代码的依赖，后续有能力者，可以自行实现功能替换开源代码

### 3.moudle通信框架
1.多module设计，暂定zc_sys,zc_codec,zc_rtsp,(后续zc_webs,zc_ui)多个进程。模块解耦 \
2.多moudle,主进程zc_sys提供注册服务，控制/设置命令直接下发到对应module,状态变更,通过订阅消息分发到不同module. \
3.moudle之间面向消息协议编程，只依赖于制订的协议. \
4.module之间消息使用nng库req/rep,pub/sub通信模型进程通信 \
5.其他开发者可以增加进程进行开发对接不同协议 \
6.zy_sys主模块，支持publish发布消息，其他子模块 subscriber接收消息 \
(暂时支持: \
1.其他模块register上之后, sys publish 发布消息给其他各个子模块 \
2.streaminfo 流信息修改之后，sys publish发布消息给其他各个子模块 \
)

(暂时考虑多进程)分布式机制，注册服务 \
原因：考虑其他开发者可以增加进程进行开发对接不同协议。而不影响其他进程运行稳定性。

多进程模式:设计进程间通信，模块与模块项目解耦；框架复杂，带来额外的系统资源 \
单个业务模块崩溃，不影响其他的业务模块，以及整个系统的稳定行

TODO:nng客户端加锁
1.bug:zc_sys进程启动之后，重新编译工程，重新执行其他mod，导致正在运行的的zc_sys段错误 "nng:poll:epoll" received signal SIGSEGV, Segmentation fault.(原因comm库文件有变化导致)

#### 3.1 mod模块设计
register注册机制
1.其他子模块sysmodule启动后主动注册到sysmodule; \
2.子模块sysmodule注册/注销上之后，sysmodule广播到其他各个模块 \
3.sysmodule提供获取已经注册上的子模块sysmodule的获取列表 \
4.sub-pub通信机制实现

TODO:
1.sysmodule负责看护注册上的子模块sysmodule(注册时提供参数可选) \
2.提供注册证书校验/机制;注册有效期配置... \
4.streammgr流fifo管理机制接口实现

### 4.跨进程音视频fifo设计
1.跨进程共享buf,如何减少拷贝？ \
2.借鉴kfifo思想实现环形fifo \
3.写入者不判断读指针位置，每个读者自行保留读指针位置 \
4.创建shmfifo写着时，同时打开一个FIFO管道,配合epoll,用于写入帧数据时，实现触发可读fifo可read事件，实现事件驱动read数据 \
5.写入帧数据时,先写入zc_frame_t帧头，在通过回调函数追加写入帧数据(可多次调用PutAppending多次追加数据，最后一包设置end标识)


2.待完善
TODO:
1.出现zc_testwriter 异常崩溃bug;暂时未找到问题，x64环境下，重启wsl之后，正常; 怀疑点memcpy地址不对齐问题？

### 5.rtsps协议
实现了rtspserver,rtspcli rtsppushserver,rtsppushcli模块 \
对应测试进程
zc_rtspcli:rtsp客户端，pull rtsp流写入到共享内存中/tmp/shmfifo_pull_v0[1] \
zc_rtsppushs:rtsppushs服务器，客户端推送上来大的流，写入到共享内存中/tmp/shmfifo_push_v0[1] \
zc_rtsppushcli：rtsp推流客户端，将/tmp/shmfifo_video0[1]中的推送到rtsp服务器 \
zc_rtsp:rtsp流服务器,实时流服务,推送上来的流,cli拉取的流,进行流转发

测试方法
```
#1.启动sys
./zc_sys
#2.启动
#3.启动rtsp服务器
./zc_rtsp
#4.启动rtsp push服务器
./zc_rtsppushs
#5.启动push客户端，将live0视频推送上push服务器上push.ch0通道上
./zc_rtsppushc rtsp://192.168.1.166:5540/push/push.ch0 0 tcp
./zc_rtsppushc rtsp://192.168.1.166:5540/push/push.ch1 1 tcp
#6.启动cli客户端，拉live.ch0实时视频，拉到pull.ch0通道上
./zc_rtspcli rtsp://192.168.1.166:8554/live/live.ch0 0 tcp
./zc_rtspcli rtsp://192.168.1.166:8554/live/live.ch1 1 tcp
#vlc取流测试
rtsp://192.168.1.166:8554/live/live.ch0
rtsp://192.168.1.166:8554/live/live.ch1
rtsp://192.168.1.166:8554/live/push.ch0
rtsp://192.168.1.166:8554/live/push.ch1
rtsp://192.168.1.166:8554/live/pull.ch0
rtsp://192.168.1.166:8554/live/pull.ch1

#启动rtspserver;默认监听端口8554;流地址rtsp://192.168.1.166:8554/live/live.ch0[1];
zc_rtsp &
##说明：共享内存的视频 /tmp/shmfifo_video0;/tmp/shmfifo_video1 对应rtsp服务器live.ch0/live.ch1

#启动推流服务器;默认端口5540 rtsp://192.168.1.166:8554/live/live.ch0
zc_rtsppushs &

#启动rtspcli,参数:zc_rtspcli 拉取流url 0(通道) [tcp|udp]
zc_rtspcli rtsp://192.168.1.66:8554/live/live.ch0 1 tcp

#启动rtsppushcli,参数:zc_rtsppushcli 推流url 0(送流通道) [tcp|udp]
rtsppushcli rtsp://192.168.1.66:5540/live/push.ch0 1 tcp
##说明：送流通道0/1，共享内存的视频 /tmp/shmfifo_video0;/tmp/shmfifo_video1 推送出去

#测试拉取实时流
vlc/ffmpeg: rtsp://192.168.1.166:8554/live/live.ch1
#测试拉取推送上去的流，
vlc/ffmpeg: rtsp://192.168.1.166:8554/live/push.ch1
#测试拉取rtspcli pull到的流，
vlc/ffmpeg: rtsp://192.168.1.166:8554/live/pull.ch1

# 如何向共享内存中写流数据，/tmp/shmfifo_video0
1.修改海思sample_vio/sample_venc
2.或者使用zc_testwriter测试程序，默认读取test.h265/test.h264文件写入到 /tmp/shmfifo_video0[1]

```

## build
### build for linux x64 gcc/g++
# x64编译环境
sudo apt install pkg-config cmake bear gcc g++
sudo apt install ffmpeg libavformat-dev libavcodec-dev libswresample-dev libswscale-dev libavutil-dev libavdevice-dev
sudo apt-get install openssl libssl-dev
thridparty/install中上传的三方库编译由
ubuntu22.04, 编译环境要求glibc2.34以上
```
#uname -a
Linux lubancat-vm 5.15.0-69-generic #76~20.04.1-Ubuntu SMP
ldd --version
ldd (Ubuntu GLIBC 2.35-0ubuntu3.8) 2.35
```

```
git clone git@github.com:Chancyn/ZeroCamera_demo.git;
cd ZeroCamera_demo;
./build.sh
```

```
测试编译环境ubuntu20.04 编译报错
undefined reference to `pthread_mutexattr_init@GLIBC_2.34'
#uname -a
Linux lubancat-vm 5.15.0-69-generic #76~20.04.1-Ubuntu SMP
#查看系统glibc版本
strings /lib/x86_64-linux-gnu/libc.so.6 |grep GLIBC_
glibc 2.30
需要升级系统glibc到2.34
#解决办法升级glibc
##修改软件源
sudo vi /etc/apt/sources.list 最后添加
deb http://th.archive.ubuntu.com/ubuntu jammy main    #添加该行到文件

##升级
sudo apt update
sudo apt install libc6
#sudo apt-get install glibc-doc manpages-posix-dev
strings /lib/x86_64-linux-gnu/libc.so.6 |grep GLIBC_
或者使用ldd --version 查看glibc是否升级到2.34以上
```



### build for linux hi3519dv500 gcc/g++
```
git clone git@github.com:Chancyn/ZeroCamera_demo.git;
cd ZeroCamera_demo;
./build.sh soc=hi3519dv500
```


## 编译第三方库
```
cd thirdparty/nng;
./build_x64.sh
cd thirdparty/spdlog;
./build_x64.sh
```


```
如需自行编译media-server
git clone git@github.com:Chancyn/ZeroCamera_demo.git; #编译
git submodule init thirdparty/package/media_server/media-server
git submodule update
#进入media_server目录编译media_server
cd thirdparty/package/media_server
#
git submodule init thirdparty/package/media_server/media-server
./build_x64.sh
返回根目录编译项目
cd -
./build_x64.sh
```

### build for rk3588 ubuntu22.04 gcc/g++
# x64编译环境
sudo apt install pkg-config cmake bear gcc g++
sudo apt install ffmpeg libavformat-dev libavcodec-dev libswresample-dev libswscale-dev libavutil-dev libavdevice-dev
sudo apt-get install openssl libssl-dev
thridparty/install中上传的三方库编译由
ubuntu22.04, 编译环境要求glibc2.34以上


### 编译option
WITH_ASAN 内存泄漏检测ASAN工具

##
参考引用:
1.openhisilicon
2.media-server
3.nng

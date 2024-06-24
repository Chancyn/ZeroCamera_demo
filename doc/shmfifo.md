# shmfifo设计思想

## 1.简介
目标：如何实现跨进程共享音视频帧数据？

实现细节
1.跨进程共享buf,如何减少拷贝？ \
2.借鉴kfifo思想实现环形fifo \
3.写入者不判断读指针位置，每个读者自行保留读指针位置 \
4.创建shmfifo写着时，同时打开一个FIFO管道,配合epoll,用于写入帧数据时，实现触发可读fifo可read事件，实现事件驱动read数据 \
5.写入帧数据时,先写入zc_frame_t帧头，在通过回调函数追加写入帧数据(可多次调用PutAppending多次追加数据，最后一包设置end标识) \
6.fifo采用循环覆盖形式，一个读者read消费fifo数据慢，read位置会被write覆盖 \
7.视频帧shmstream设计有magic头，确保被覆盖时候能够找到正确的帧数据 (当前策略视频帧fifo会找到最新的key帧) \
8.跨进程的互斥锁实现 \
9.进程独立性：如何实现读者或者写者进程启动顺序无前后依赖关系影响。\
10.进程独立性：如何保证 读者/写着进程奔溃 时，程序不受影响，跨进程锁不会导致死锁，崩溃进程重启后依旧正常工作。

## 2.待完善功能
TODO:
1.如何实现读端如何知道FIFO中的帧编码格式类型H264/H265? 写段编码格式切换，读端同步? (增加shmstream管理类统一管理) \
2.满足录制需求，实现预录制功能；需实现方向查找找到fifo中最老的I帧，实现预录制功能，当前打开流时默认找最新的I帧; \
3.满足回放需求，点播形式，读端需要阻塞写端。1读1写 (当前fifo适用于实时视频1写多读)

## 3.主要功能类
CShmFIFO: 字节流跨进程共享fifo 基类 \
CShmFIFOW: 读端FIFO派生类 \
CShmFIFOR: 写端FIFO派生类

### 3.1主要接口
ShmAlloc:alloc 共享内存 \
ShmFree:释放共享内存 \
Put:写入数据FIFO接口 \
Get:读取FIFO中数据接口 \
GetEvFd: evfifo的fd,搭配epoll,实现事件驱动，读操作.

#### 3.1.1用法参考
tests/zc_test_shmfifoev.cpp中示例代码使用

### 3.2帧数据共享FIFO
CShmStream: 继承CShmFIFO，实现帧数据流跨进程共享fifo;保证每次写入/读出操作时，都为完整的1帧数据 \
CShmStreamW: 读端FIFO派生类 \
CShmStreamR: 写端FIFO派生类

ShmAlloc:alloc 共享内存 \
ShmFree:释放共享内存 \
Put:写入帧数据FIFO接口 \
Get:读取FIFO中帧数据接口 \
GetEvFd: evfifo的fd,搭配epoll,实现事件驱动，读操作.

#### 3.2.1用法参考
写者参考:src/media/zcrtsp/CLiveTestWriterH264.cpp中示例代码使用 \
读者参考:src/media/zcrtsp/ZcMediaTrack.cpp中示例代码使用

## 4.C-API
封装了c格式api,供c语言调用

### 4.1参考用法
doc/hisi_sample/sample_comm_venc.c中示例使用

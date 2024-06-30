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


## 5.遗留问题
### 5.1 遗留bug

```
[20240630_10_10_41_587][2024-06-30 10:10:41.580][warning][tid 2615312] [ZcLiveTestWriterH264.cpp 151][fillnaluInfo]fillSdpInfo num:1, type:2, size:28
[20240630_10_10_41_589]68 ee 31 b2 00 00 00 01 06 e5 01 f3 80 00 00 00 01 65 b8 00 00 2b 0e 00 00 1b ff d2
[20240630_10_10_44_320]
[20240630_10_10_44_320]Thread 3 "TestWH264_1" received signal SIGSEGV, Segmentation fault.
[20240630_10_10_44_321][Switching to Thread 0x7ffff5eed640 (LWP 2615312)]
[20240630_10_10_44_349]__memmove_avx_unaligned_erms () at ../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S:708
[20240630_10_10_44_350]708     ../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S: No such file or directory.
[20240630_10_16_13_104](gdb) bt
[20240630_10_16_13_235]#0  __memmove_avx_unaligned_erms () at ../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S:708
[20240630_10_16_13_367]#1  0x00005555555631ba in zc::CShmFIFO::_put (this=0x555555588740, buffer=0x7fffd93b0135 "", len=14590)
[20240630_10_16_13_381]    at /home/zhoucc/work/study/zc_demo/ZeroCamera_demo/src/bsw/fifo/ZcShmFIFIO.cpp:379
[20240630_10_16_13_393]#2  0x0000555555563f71 in zc::CShmStreamW::PutAppending (this=0x555555588740, buffer=0x7fffd93b0135 "", len=14590)
[20240630_10_16_13_407]    at /home/zhoucc/work/study/zc_demo/ZeroCamera_demo/src/bsw/fifo/ZcShmStream.cpp:78
[20240630_10_16_13_410]#3  0x000055555555b5b5 in zc::CLiveTestWriterH264::_putingCb (this=0x555555587dc0, stream=0x7ffff5eecc20)
[20240630_10_16_13_411]    at /home/zhoucc/work/study/zc_demo/ZeroCamera_demo/src/media/zcrtsp/media/ZcLiveTestWriterH264.cpp:48
[20240630_10_16_13_416]#4  0x000055555555b5e7 in zc::CLiveTestWriterH264::putingCb (u=0x555555587dc0, stream=0x7ffff5eecc20)
[20240630_10_16_13_427]    at /home/zhoucc/work/study/zc_demo/ZeroCamera_demo/src/media/zcrtsp/media/ZcLiveTestWriterH264.cpp:53
[20240630_10_16_13_473]#5  0x0000555555563f24 in zc::CShmStreamW::Put (this=0x555555588740, buffer=0x7ffff5eecc30 "EVCZ\376\070", len=68, stream=0x7ffff5eecc20)
[20240630_10_16_13_487]    at /home/zhoucc/work/study/zc_demo/ZeroCamera_demo/src/bsw/fifo/ZcShmStream.cpp:67
[20240630_10_16_13_561]#6  0x000055555555b985 in zc::CLiveTestWriterH264::_putData2FIFO (this=0x555555587dc0)
[20240630_10_16_13_564]    at /home/zhoucc/work/study/zc_demo/ZeroCamera_demo/src/media/zcrtsp/media/ZcLiveTestWriterH264.cpp:112
[20240630_10_16_13_580]#7  0x000055555555bf07 in zc::CLiveTestWriterH264::process (this=0x555555587dc0)
[20240630_10_16_13_582]    at /home/zhoucc/work/study/zc_demo/ZeroCamera_demo/src/media/zcrtsp/media/ZcLiveTestWriterH264.cpp:179
[20240630_10_16_13_587]#8  0x000055555556507a in zc::Thread::_run (this=0x555555587dc0) at /home/zhoucc/work/study/zc_demo/ZeroCamera_demo/src/bsw/utilsxx/Thread.cpp:68
[20240630_10_16_13_610]#9  0x0000555555565d4d in std::__invoke_impl<void, void (zc::Thread::*)(), zc::Thread*> (
[20240630_10_16_13_647]    __f=@0x555555586b60: (void (zc::Thread::*)(zc::Thread * const)) 0x555555564fa2 <zc::Thread::_run()>, __t=@0x555555586b58: 0x555555587dc0)
[20240630_10_16_13_649]    at /usr/include/c++/11/bits/invoke.h:74
[20240630_10_16_13_652]#10 0x0000555555565c9f in std::__invoke<void (zc::Thread::*)(), zc::Thread*> (
[20240630_10_16_13_655]    __fn=@0x555555586b60: (void (zc::Thread::*)(zc::Thread * const)) 0x555555564fa2 <zc::Thread::_run()>) at /usr/include/c++/11/bits/invoke.h:96
[20240630_10_16_13_656]#11 0x0000555555565bff in std::thread::_Invoker<std::tuple<void (zc::Thread::*)(), zc::Thread*> >::_M_invoke<0ul, 1ul> (this=0x555555586b58)
[20240630_10_16_13_657]    at /usr/include/c++/11/bits/std_thread.h:259
[20240630_10_16_13_660]#12 0x0000555555565bb4 in std::thread::_Invoker<std::tuple<void (zc::Thread::*)(), zc::Thread*> >::operator() (this=0x555555586b58)
[20240630_10_16_13_663]    at /usr/include/c++/11/bits/std_thread.h:266
[20240630_10_16_13_684]#13 0x0000555555565b94 in std::thread::_State_impl<std::thread::_Invoker<std::tuple<void (zc::Thread::*)(), zc::Thread*> > >::_M_run (
[20240630_10_16_13_687]    this=0x555555586b50) at /usr/include/c++/11/bits/std_thread.h:211
[20240630_10_16_13_693]#14 0x00007ffff7c06253 in ?? () from /lib/x86_64-linux-gnu/libstdc++.so.6
[20240630_10_16_13_793]#15 0x00007ffff7975ac3 in start_thread (arg=<optimized out>) at ./nptl/pthread_create.c:442
[20240630_10_16_13_807]#16 0x00007ffff7a07850 in clone3 () at ../sysdeps/unix/sysv/linux/x86_64/clone3.S:81
[20240630_10_16_20_655](gdb) bt full f 2
[20240630_10_16_20_666]#0  __memmove_avx_unaligned_erms () at ../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S:708
```
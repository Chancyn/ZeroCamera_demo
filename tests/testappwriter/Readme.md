# zc_testwriter进程测试说明

## 功能简介
1.读取视频H264/H265文件, test0.h265/test1.h264文件，并写入到共享直播live streamfifo中 \
2.用于模拟encode 模块编码写入fifo文件 \
3.最大支持两个通道2通道 \
4.支持修改通道编码参数变更后，发送setstreaminfo消息到zc_sys模块更改编码信息，rtspserver会自适性修改live通道的编码参数

## 测试参数
```
#首先先启动zc_sys和zc_rtsp
./zc_sys &
./zc_rtsp &

#默认0通道读取test0.H265,1通道test1.H264
./zc_testwriter

#默认0通道读取test0.H264,1通道test1.H265
./zc_testwriter h264 h265
```
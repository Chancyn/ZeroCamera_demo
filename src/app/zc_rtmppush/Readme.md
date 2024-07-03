# rtspcli
rtspcli 拉流端测试进程

## 执行方式
执行
./zc_rtspcli rtsp://192.168.1.66:8554/live/live.ch1 [tcp|udp]

示例
udp取流
./zc_rtspcli rtsp://192.168.1.66:8554/live/live.ch1 udp
tcp取流
./zc_rtspcli rtsp://192.168.1.66:8554/live/live.ch1 tcp
默认 tcp方式

bug:
1.udp方式下拉流测试,ctrl+c无法退出程序

TODO:(zhoucc)
后续待完成
1.多线程，多个rtspcli客户端支持，用于做rtsp测试
2.私有格式raw数据支持

##
RTMP push客户端

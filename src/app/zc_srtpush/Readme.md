# rtmppush
rtmppush 推流端测试进程

## 执行方式
执行
./zc_srtpush srt://192.168.1.166/live/livestream chn

示例
rtmp推流测试
./zc_srtpush srt://192.168.1.166/live/livestream 1

TODO:(zhoucc)
后续待完成
1.aac测试
2.bug [error][tid 1479158] [ZcFlvMuxer.cpp 148][_packetFlv]flv_muxer_avc err.
原因，两个rtmppush端推不同流格式到同一个push地址报错;

## 测试命令
RTMP push客户端
```
# 测试
#启动sys
./zc_sys
# 启动读取流
./zc_testwriter
# 启动srs rtmp服务器(见rtmp服务器)
./objs/srs -c conf/srs.conf
# 启动rtmppush 将实时流1(H264)流推送到服务器
./zc_srtpush rtmp://192.168.1.166/live/livestream 1
# vlc拉流测试(仅支持H264拉流)
rtmp://192.168.1.166/live/livestream
# srs http-flv测试页面打开
http://192.168.1.166:8080/players/srs_player.html
# srs http-flv测试页面打开(支持H264/H265拉流)
http://192.168.1.166:8080/live/livestream.flv
# 测试H265 rtmppush 将实时流0(H265)流推送到服务器 livestream0
./zc_srtpush rtmp://192.168.1.166/live/livestream0 0

```
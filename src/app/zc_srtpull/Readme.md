# rtmppull
rtmppull rtmp pull client 测试进程

## 执行方式
执行
./zc_srtpull 'srt://192.168.1.166:10080?streamid=#!::r=live/livestream,m=request' 1

示例
rtmp推流测试
./zc_srtpull -p 1935 port

TODO:(zhoucc)
后续待完成
原因，两个rtmppush端推不同流格式到同一个push地址报错;

## 测试命令
RTMP pull客户端,实现 rtmppull,支持拉去rtmpserver服务流
```
# 测试
#启动sys
./zc_sys
# 启动读取流
./zc_testwriter
# 启动rtmp服务器(见rtmp服务器)
./zc_rtmpsvr

# 启动rtmppush 将实时流1(H264)流推送到服务器
./zc_rtmppush rtmp://192.168.1.166/live/1.live 1
#使用ffmpeg推流测试
ffmpeg -re -i test_1080p60.h264.mp4 -c copy -f flv rtmp://192.168.1.166/live/1.live

# vlc拉流测试(仅支持H264拉流)
rtmp://192.168.1.166/live/1.live.flv


# 启动webs http-flv测试页面打开
./zc_webs

# srs http-flv测试页面打开(支持H264/H265拉流)
http://192.168.1.166:8080/live/livestream.flv
# 测试H265 rtmppush 将实时流0(H265)流推送到服务器 livestream0
./zc_rtmppush rtmp://192.168.1.166/live/0.live 0

# 测试H265 rtmppush 将实时流0(H265)流推送到服务器 livestream0
./zc_srtpull rtmp://192.168.1.166/live/0.live 0

```
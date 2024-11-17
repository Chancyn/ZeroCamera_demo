# srtpush
srtpush 推流端测试进程

## 执行方式
执行
./zc_srtpush srt://192.168.1.166/live/livestream chn

示例
### ffmpeg+srs推流测试
启动srs服务器
./objs/srs -c conf/srt.conf
启动ffmpeg推流srt
ffmpeg -re -i test_1080p60.h264 -c copy -pes_payload_size 0 -f mpegts 'srt://127.0.0.1:10080?streamid=#!::r=live/livestream,m=publish'
VLC播放地址srs实现srt转rtmp
rtmp://192.168.1.166/live/livestream
srtpush 推流测试 H264
./zc_srtpush "srt://192.168.1.166:10080?streamid=#h=live/livestream,m=publish" 1
rtmp://192.168.1.166/live/livestream

TODO:(zhoucc)
1.H265测试验证

## 测试命令
srtpush客户端
```
# 测试
#启动sys
./zc_sys
# 启动读取流
./zc_testwriter
# 启动srs rtmp服务器(见rtmp服务器)
./objs/srs -c conf/srs.conf

srt://192.168.1.166:10080?streamid=#!::h=live/livestream,m=publish 0
# 启动zc_srtpush 将实时流1(H264)流推送到服务器
./zc_srtpush srt://192.168.1.166:10080?streamid=#!::h=live/livestream,m=publish 0
# vlc拉流测试(仅支持H264拉流)
rtmp://192.168.1.166/live/livestream
# srs http-flv测试页面打开
http://192.168.1.166:8080/players/srs_player.html
# srs http-flv测试页面打开(支持H264/H265拉流)
http://192.168.1.166:8080/live/livestream.flv
# 测试H265 rtmppush 将实时流0(H265)流推送到服务器 livestream0
./zc_srtpush rtmp://192.168.1.166/live/livestream0 0
```
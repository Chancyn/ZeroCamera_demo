# webs 模块
webs 服务器模块

## 支持http/https
1.测试http/https
```
#执行zc_sys
./zc_sys
#拷贝webs服务器测试资源 mongoose_test_demo
#执行zc_webs
./zc_webs
#测试http/https
http://192.168.1.166:80
https://192.168.1.166:8443
```

## 支持ws/wss
```
#输入ws测试地址
http://192.168.1.166:80/test.html
#输入 ws服务器地址，点击连接测试
ws://localhost:8080/websocket
#输入字符串发送aaa bbb
CONNECTION OPENED
SENT: aaa
RECEIVED: aaa
SENT: bbb
RECEIVED: bbb
或者直接连接http端口测试
ws://localhost:8000/websocket
```

## http-flv
1.支持http-flv
测试方法
```
打开页面
http://192.168.1.166:8000/html/players/srs_player.html
选择http-flv播放输入地址播放
http://192.168.1.166:8000/live/livestream.flv

```

## flv.js测试
a. 测试方法ws-flv
使用flv.js播放测试,ws-flv
访问测试地址 http://192.168.1.166:8000/flv.js/demo/index.html
使用 MeidaDataSource配置ws-flv.json文件
http://192.168.1.166:8000/flv/ws-flv.json
```
{
	"type": "flv",
	"isLive": true,
	"url": "ws://192.168.1.166:8000/wslive/live.flv"
}
```

b. 测试方法http-flv
使用flv.js播放测试,http-flv
访问测试地址 http://192.168.1.166:8000/flv.js/demo/index.html
使用 MeidaDataSource配置http-flv.json文件
http://192.168.1.166:8000/flv/http-flv.json
```
{
	"type": "flv",
	"isLive": true,
	"url": "http://192.168.1.166:8000/live/live.flv"
}
```



## 完成
2.http-flv，出错关闭逻辑，清除http-live-session;
3.http-flv 浏览器播放几十秒之后停止(问题原因发送命令)

##
TODO:
1.websocket-flv


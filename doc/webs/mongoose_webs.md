# mongoose webserver
##
Mongoose 是一款嵌入式 Web 服务器库，具有跨平台、轻量级、支持多种网络协议、稳定可靠等特点
官方开发参考文档
https://mongoose.ws/documentation/tutorials/tls/#tls-for-servers

## 测试
### 1.mongoose 测试demo
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

2.测试websocket
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

2.测试wss websocket ssl
mongoose测试页面
```
输入ws测试地址
http://192.168.1.166:8443/test.html
输入 ws服务器地址，连接测试
wss://localhost:8444/websocket
测试报错
ERROR: [object Event]
CONNECTION CLOSED
日志
4096B395567F0000:error:0A000416:SSL routines:ssl3_read_bytes:sslv3 alert certificate unknown:../ssl/record/rec_layer_s3.c:1584:SSL alert number 46
服务器发送的证书在客户端看来是未知的或不信任的
```
测试客户端存在问题

使用在线网站测试
```
http://x-tool.online/websocket
输入 ws服务器地址，连接测试
wss://localhost:8444/websocket
测试成功
或者直接连接https端口测试
wss://localhost:8443/websocket
```

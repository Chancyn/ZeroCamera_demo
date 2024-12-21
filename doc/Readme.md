# readme
cmake测试工程

## 1.工程目录
```
.
├── thirdparty                 #第三方库
├── build                   #cmake编译目录
│   ├── hi3519dv500
│   └── x86_64                 #x86_64平台
├── doc                     #文档
│   └── cmake               #示例CMakelist.txt
├── firmware                #打包
├── inc                     #公用头文件
├── output                  #编译输出文件
│   ├── hi3519dv500
│   └── x86_64
└── src                     #源码
    ├── app                 #app文件
    ├── bsw                 #bsw基础软件
    └── plat                #平台层接口
```
## 编译
### 编译第三方库
example nng
```
cd thirdparty/thirdparty/nng;
./build_hi3519dv500.sh
or ./build_x86_64.sh
```
### 编译代码
./build.sh soc=hi3519dv500
or ./build.sh
## 运行
### x86运行环境变量
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/zhoucc/work/study/cmake/test/output/x86_64/lib
./zc_app
### hi3519dv00运行环境
拷贝动态库到板端如/usr/local/lib,设置环境变量
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
./zc_app
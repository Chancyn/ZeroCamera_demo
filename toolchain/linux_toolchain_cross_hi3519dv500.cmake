#其目的是设置目标机使用的操作系统名称，支持Linux，QNX，WindowsCE，Android等。
SET(CMAKE_SYSTEM_NAME Linux) #设置系统类型
#嵌入式系统环境镜像根目录，将其加到其他搜索目录之前
# SET(CMAKE_FIND_ROOT_PATH  /opt/linux/x86-arm/aarch64-v01c01-linux-gnu-gcc)
# set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER aarch64-v01c01-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-v01c01-liaanux-gnu-g++)

set(CMAKE_SYSTEM_PROCESSOR aarch64)
# set(CMAKE_C_COMPILER /opt/linux/x86-arm/aarch64-v01c01-linux-gnu-gcc/bin/aarch64-v01c01-linux-gnu-gcc)
# set(CMAKE_CXX_COMPILER /opt/linux/x86-arm/aarch64-v01c01-linux-gnu-gcc/bin/aarch64-v01c01-linux-gnu-g++)
set(CMAKE_C_COMPILER aarch64-v01c01-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-v01c01-liaanux-gnu-g++)

#从来不在指定目录下查找工具程序
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#只在指定目录下查找库文件
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
#只在指定目录下查找头文件
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
#只在指定目录下查找包
#SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

set(ZC_SOC $ENV{soc})

set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
set(OPENSSL_LIBRARY_DIR ${OPENSSL_ROOT_DIR}/lib64)

option(WITH_FFMPEG "ffmpeg option"     OFF)
option(WITH_OPENSSL "openssl option"   ON)
option(WITH_ASAN "asan option" OFF)
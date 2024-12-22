# 设置目标系统的名称（对于 x64 Linux，这通常是 Linux，但也可以是其他操作系统名称）
set(CMAKE_SYSTEM_NAME Linux)

# 设置目标系统的处理器架构为 x86_64（即 x64）
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(ZC_SOC $ENV{soc})

if(USE_OPENSSL_PC)
find_package(OpenSSL REQUIRED)
else()
set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
set(OPENSSL_LIBRARY_DIR ${OPENSSL_ROOT_DIR}/lib)
endif()

option(WITH_FFMPEG "ffmpeg option"     OFF)
option(WITH_OPENSSL "openssl option"   ON)
option(WITH_ASAN "asan option" OFF)

# media server
add_definitions(-DOS_LINUX)
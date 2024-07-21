cmake_minimum_required(VERSION 3.13)

project(zc_test VERSION 0.1.0 LANGUAGES C CXX)

#set the project name
message("build $ENV{soc}")
set(ZC_SOC $ENV{soc})

#install
set(ZC_BIN_INSTALL ${PROJECT_SOURCE_DIR}/output/${ZC_SOC}/bin)
set(ZC_LIB_INSTALL ${PROJECT_SOURCE_DIR}/output/${ZC_SOC}/lib)

set(CMAKE_SYSTEM_NAME Linux)    #设置目标系统名字

#soc empty
set (CFLAGS_SOC )
set (CXXFLAGS_SOC )

# set platform
if("${ZC_SOC}x" STREQUAL "hi3519dv500x" )
message("build hi3519dv500 ####")
set(COMPILER_PREFIX "aarch64-v01c01-linux-gnu-")
set(CMAKE_SYSTEM_PROCESSOR arm)
set (CFLAGS_SOC -mcpu=cortex-a55)
set (CXXFLAGS_SOC -mcpu=cortex-a55)
elseif("${ZC_SOC}x" STREQUAL "ss928v100x" )
message("build ss928v100 ####")
set(COMPILER_PREFIX "aarch64-mix210-linux-")
set(CMAKE_SYSTEM_PROCESSOR arm)
elseif("${ZC_SOC}x" STREQUAL "rk3588x" )
message("build rk3588 ####")
set(COMPILER_PREFIX "aarch64-none-linux-gnu-")
set(CMAKE_SYSTEM_PROCESSOR arm)

#ffmpeg
set(FFMPEG_INC include)
set(FFMPEG_LIB lib)
else()
message("build ${ZC_SOC} ####")
#build memoryleak detect tool ON
option(WITH_ASAN "asan option" OFF)
endif()


set (CMAKE_C_COMPILER ${COMPILER_PREFIX}gcc)
set (CMAKE_CXX_COMPILER ${COMPILER_PREFIX}g++)
message("build #### C compiler:${CMAKE_C_COMPILER} ####")
message("build #### CXX compiler:${CMAKE_CXX_COMPILER} ####")
set(CMAKE_C_FLAGS
    "${CFLAGS_SOC} -Wall -std=gnu11 -fPIC -pthread"
)

#"${CXXFLAGS_SOC} -Wall -std=c++11 -Werror" -std=c++11
set(CMAKE_CXX_FLAGS
    "${CXXFLAGS_SOC} -Wall -std=gnu++11 -fPIC -pthread"
)

if(WITH_ASAN)
    add_compile_options(-fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer)
endif()

#build cmd debug/release
if(NOT CMAKE_BUILD_TYPE)
message("build not define CMAKE_BUILD_TYPE")
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")
message("build default " ${CMAKE_BUILD_TYPE})
else()
message("build with " ${CMAKE_BUILD_TYPE})
endif()

# ZC_LIVE_TEST test read h264file,and write to fifo
add_definitions(-DZC_LIVE_TEST=1)
# ZC_LIVE_TEST test read h264file,rtps write shmstreambuff, not process shared,
add_definitions(-DZC_LIVE_TEST_THREADSHARED=0)

if("${CMAKE_BUILD_TYPE}x" STREQUAL "Debugx")
       #SET(CMAKE_ZC_DEBUG 1)
       message("build with Debug")
       add_definitions(-DZC_DEBUG)
       #add_definitions(-DSPDLOG_ACTIVE_LEVEL=0)
else()
       #SET(CMAKE_ZC_DEBUG 0)
       message("build with Release")
endif()

option(WITH_OPENSSL "openssl option"     ON)
if(WITH_OPENSSL)
set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC})
find_package(OpenSSL REQUIRED)
# mongoose ssl mg
add_definitions(-DMG_TLS=MG_TLS_OPENSSL)
endif()

#media server
add_definitions(-D__ERROR__=00*10000000+__LINE__*1000)


set(SPDLOG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/spdlog/include)
set(SPDLOG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/spdlog/lib)
set(NNG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/include)
set(NNG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/lib)

set(PLAT_BASIC_INC ${CMAKE_SOURCE_DIR}/src/plat/basic)
set(MSGCOMM_INC ${CMAKE_SOURCE_DIR}/src/bsw/msgcomm)
set(UTILSXX_INC ${CMAKE_SOURCE_DIR}/src/bsw/utilsxx)

set(ZCRTSP_INC ${CMAKE_SOURCE_DIR}/src/media/zcrtsp)
set(ZCCODEC_INC ${CMAKE_SOURCE_DIR}/src/media/zccodec)

set(ZCFLV_INC ${CMAKE_SOURCE_DIR}/src/media/zcflv)
set(ZCRTMP_INC ${CMAKE_SOURCE_DIR}/src/media/zcrtmp)

set(ZCFMP4_INC ${CMAKE_SOURCE_DIR}/src/media/zcfmp4)

set(MEDIAINTALL ${CMAKE_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/media_server)
set(MEDIARTSPAIO rtspaio)
set(MEDIARTSPMEDIA rtspmedia)
set(MEDIAFILEBUF movfilebuf)
# rtsp
set(MEDIARTSP rtsp)
set(MEDIARTSP_LIBPATH ${MEDIAINTALL}/media-server/librtsp/lib)
set(MEDIARTSP_INC ${MEDIAINTALL}/media-server/librtsp/include)
#set(MEDIARTSP_INC2 ${MEDIAINTALL}/media-server/librtsp/source/server)

#rtmp
set(MEDIARTMP rtmp)
set(MEDIARTMP_LIBPATH ${MEDIAINTALL}/media-server/librtmp/lib)
set(MEDIARTMP_INC ${MEDIAINTALL}/media-server/librtmp/include)

set(MEDIARTP rtp)
set(MEDIARTP_LIBPATH ${MEDIAINTALL}/media-server/librtp/lib)
set(MEDIARTP_INC ${MEDIAINTALL}/media-server/librtp/include)

set(MEDIAFLV flv)
set(MEDIAFLV_LIBPATH ${MEDIAINTALL}/media-server/libflv/lib)
set(MEDIAFLV_INC ${MEDIAINTALL}/media-server/libflv/include)

set(MEDIAMOV mov)
set(MEDIAMOV_LIBPATH ${MEDIAINTALL}/media-server/libmov/lib)
set(MEDIAMOV_INC ${MEDIAINTALL}/media-server/libmov/include)

set(MEDIAMPEG mpeg)
set(MEDIAMPEG_LIBPATH ${MEDIAINTALL}/media-server/libmpeg/lib)
set(MEDIAMPEG_INC ${MEDIAINTALL}/media-server/libmpeg/include)

set(MEDIAMKV mkv)
set(MEDIAMKV_LIBPATH ${MEDIAINTALL}/media-server/libmkv/lib)
set(MEDIAMKV_INC ${MEDIAINTALL}/media-server/libmkv/include)

set(MEDIASDK sdk)
set(MEDIASDK_LIBPATH ${MEDIAINTALL}/sdk/libsdk/lib)
set(MEDIASDK_INC ${MEDIAINTALL}/sdk/include)

set(MEDIAAVCODEC avcodec)
set(MEDIAAVCODEC_INC ${MEDIAINTALL}/avcodec/avcodec/include)
set(MEDIAAVCODEC_LIBPATH ${MEDIAINTALL}/avcodec/avcodec/lib)

set(MEDIAAVBSF avbsf)
set(MEDIAAVBSF_INC ${MEDIAINTALL}/avcodec/avbsf/include)
set(MEDIAAVBSF_LIBPATH ${MEDIAINTALL}/avcodec/avbsf/lib)

set(MEDIAHTTP http)
set(MEDIAHTTP_LIBPATH ${MEDIAINTALL}/sdk/libhttp/lib)
set(MEDIAHTTP_INC ${MEDIAINTALL}/sdk/libhttp/include)
set(MEDIAAIO aio)
set(MEDIAAIO_LIBPATH ${MEDIAINTALL}/sdk/libaio/lib)
set(MEDIAAIO_INC ${MEDIAINTALL}/sdk/libaio/include)

include_directories(include
include/zc_log
include/zccrypto
include/fifo
include/utilsxx
include/utils
include/msgcomm
include/mod
include/media
include/binmsg
include/basic
)

#option build if or not
#option(WITH_BSW "bsw option"     ON)
#if(WITH_BSW)
#    TODO
#    add_definitions(-DWITH_BSW)
#endif()

#option(WITH_FFMPEG "ffmpeg option"     ON)
option(WITH_FFMPEG "ffmpeg option"     OFF)
if(WITH_FFMPEG)
    add_definitions(-DWITH_FFMPEG)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC libavcodec)
pkg_check_modules(AVFORMAT libavformat)
pkg_check_modules(AVUTIL libavutil)
pkg_check_modules(SWSCALE libswscale)
pkg_check_modules(SWRESAMPLE libswresample)
pkg_check_modules(AVDEVICE libavdevice)

if(WITH_FFMPEG)
# 包含ffmpeg头文件
include_directories(zc_testv4l2 PRIVATE
${AVDEVICE_INCLUDE_DIRS}
${AVFORMAT_INCLUDE_DIRS}
${AVCODEC_INCLUDE_DIRS}
${SWSCALE_INCLUDE_DIRS}
${SWRESAMPLE_INCLUDE_DIRS}
${AVUTIL_INCLUDE_DIRS} )
endif()

add_subdirectory(src)
add_subdirectory(tests)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


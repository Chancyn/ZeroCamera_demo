cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)
add_executable(zc_testv4l2 ${DIR_SOURCE_SRCS})
#add_executable(zc_testv4l2 zc_testv4l2_main.cpp)
foreach(SOURCE_FILE ${DIR_SOURCE_SRCS})
	get_filename_component(filename ${SOURCE_FILE} NAME)
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${filename}\""
    )
endforeach()

#add_executable(zc_tests zc_test_main.cpp)
#头文件包含
#头文件包含
target_include_directories(zc_testv4l2 PRIVATE
${ZCCODEC_INC}
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
)

#连接库
target_link_libraries(zc_testv4l2 PRIVATE
zccodec
zcfifo
zc_log

# plat_basic
# bsw_basic
utilsxx
utils
pthread
)

#asan
if(WITH_ASAN)
target_link_libraries(zc_testv4l2 PRIVATE
asan.a
)
endif()

if(WITH_FFMPEG)
# 包含ffmpeg头文件

target_include_directories(zc_testv4l2 PRIVATE
${AVDEVICE_INCLUDE_DIRS}
${AVFORMAT_INCLUDE_DIRS}
${AVCODEC_INCLUDE_DIRS}
${SWSCALE_INCLUDE_DIRS}
${SWRESAMPLE_INCLUDE_DIRS}
${AVUTIL_INCLUDE_DIRS} )

target_include_directories(zc_testv4l2 PRIVATE
${AVDEVICE_LIBRARIES_DIRS}
${AVFORMAT_LIBRARIES_DIRS}
${AVCODEC_LIBRARIES_DIRS}
${SWSCALE_LIBRARIES_DIRS}
${SWRESAMPLE_LIBRARIES_DIRS}
${AVUTIL_LIBRARIES_DIRS} )

#-lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale -lx264
# 链接ffmpeg库
target_link_libraries(zc_testv4l2 PRIVATE
${AVDEVICE_LIBRARIES}
${AVFORMAT_LIBRARIES}
${AVCODEC_LIBRARIES}
${SWSCALE_LIBRARIES}
${SWRESAMPLE_LIBRARIES}
${AVUTIL_LIBRARIES} )
endif()

enable_testing()
add_test(NAME zc_testv4l2 COMMAND Test)

#执行文件的安装位置
install(TARGETS zc_testv4l2 RUNTIME DESTINATION ${ZC_BIN_INSTALL})
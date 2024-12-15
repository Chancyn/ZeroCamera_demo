cmake_minimum_required(VERSION 3.13)
aux_source_directory(./ DIR_SOURCE_SRCS)

foreach(SOURCE_FILE ${DIR_SOURCE_SRCS})
	get_filename_component(filename ${SOURCE_FILE} NAME)
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${filename}\""
    )
endforeach()

# 指定生成目标
add_executable(zc_webs ${DIR_SOURCE_SRCS})

#头文件包含
target_include_directories(zc_webs PRIVATE
${ZCMPEG_INC}
${ZCFLV_INC}
${ZCFMP4_INC}
${MEDIAMOV_INC}
${MEDIAFLV_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
)

target_link_directories(zc_webs PRIVATE
${MEDIAMOV_LIBPATH}
${MEDIAFLV_LIBPATH}
${MEDIAMPEG_LIBPATH}
${NNG_LIBPATH}
)

#连接库
target_link_libraries(zc_webs PRIVATE
zcmpeg
zcfmp4
zcflv
zch26x
${MEDIAMOV}
${MEDIAFLV}
zcfifo
mod_base
msgcomm
plat_basic
bsw_basic
nng
zc_log
utilsxx
utils
pthread
)

if(WITH_OPENSSL)
target_link_directories(zc_webs PRIVATE
${OPENSSL_SSL_LIBRARY}
${OPENSSL_CRYPTO_LIBRARY}
)
target_link_libraries(zc_webs PRIVATE
ssl
crypto
)
endif()

#asan
if(WITH_ASAN)
target_link_libraries(zc_webs PRIVATE
asan.a
)
endif()

#执行文件的安装位置
install(TARGETS zc_webs RUNTIME DESTINATION ${ZC_BIN_INSTALL})
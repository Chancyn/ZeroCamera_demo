cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

# 指定生成目标
add_executable(zc_app ${DIR_SOURCE_SRCS})

#头文件包含
target_include_directories(zc_app PRIVATE
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
)

# target_link_directories(zc_app PRIVATE
# ${SPDLOG_LIBPATH}
# )

#连接库
target_link_libraries(zc_app PRIVATE
plat_basic
bsw_basic
zc_log
utilsxx
utils
pthread
)

#执行文件的安装位置
install(TARGETS zc_app RUNTIME DESTINATION ${ZC_BIN_INSTALL})
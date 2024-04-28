cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

add_executable(zc_man ${DIR_SOURCE_SRCS})

target_include_directories(zc_man PRIVATE
${NNG_INC}
${ZC_LOG_INC}
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${ZC_NNG_INC}
)

target_link_directories(zc_man PRIVATE
${NNG_LIBPATH}
)

target_link_libraries(zc_man PRIVATE
plat_basic
bsw_basic
zc_log
zc_nng
nng
pthread
)

install(TARGETS zc_man RUNTIME DESTINATION ${ZC_BIN_INSTALL})
cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

#.a/so
add_library(bsw_basic STATIC ${DIR_SOURCE_SRCS})
#add_library(bsw_basic SHARED ${DIR_SOURCE_SRCS})

target_include_directories(bsw_basic PRIVATE
${ZC_LOG_INC}
)

#install(TARGETS bsw_basic RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS bsw_basic DESTINATION ${ZC_LIB_INSTALL})
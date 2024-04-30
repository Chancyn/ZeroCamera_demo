cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

#.a/so
add_library(utils STATIC ${DIR_SOURCE_SRCS})
#add_library(utils SHARED ${DIR_SOURCE_SRCS})

#target_include_directories(utils PRIVATE ${ZC_LOG_INC})

install(TARGETS utils DESTINATION ${ZC_LIB_INSTALL})
cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

#.a/so
add_library(utilsxx STATIC ${DIR_SOURCE_SRCS})
#add_library(utilsxx SHARED ${DIR_SOURCE_SRCS})

#target_include_directories(utilsxx PRIVATE ${ZC_LOG_INC})

install(TARGETS utilsxx DESTINATION ${ZC_LIB_INSTALL})
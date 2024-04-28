cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

add_library(plat_basic SHARED ${DIR_SOURCE_SRCS})

#install(TARGETS plat_basic RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS plat_basic DESTINATION ${ZC_LIB_INSTALL})
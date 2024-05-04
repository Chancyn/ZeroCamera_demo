cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

add_library(mod_base SHARED ${DIR_SOURCE_SRCS})

#install(TARGETS mod_base RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS mod_base DESTINATION ${ZC_LIB_INSTALL})
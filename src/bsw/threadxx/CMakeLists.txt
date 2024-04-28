cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

#.a/so
add_library(threadxx STATIC ${DIR_SOURCE_SRCS})
#add_library(threadxx SHARED ${DIR_SOURCE_SRCS})

target_include_directories(threadxx PRIVATE
${ZC_LOG_INC}
)

install(TARGETS threadxx DESTINATION ${ZC_LIB_INSTALL})
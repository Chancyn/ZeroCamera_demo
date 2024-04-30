cmake_minimum_required(VERSION 3.22)

aux_source_directory(./ SUB_DIR_SOURCE_SRCS)
#.a/so
add_library(zc_test_timer STATIC ${SUB_DIR_SOURCE_SRCS})
#add_library(zc_test_timer STATIC ./)
target_include_directories(zc_test_timer PRIVATE
${UTILSXX_INC}
${UTILS_INC}
)

#install(TARGETS zc_test_timer DESTINATION ${ZC_LIB_INSTALL})
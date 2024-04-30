cmake_minimum_required(VERSION 3.22)
# aux_source_directory(./ DIR_SOURCE_SRCS)

# #.a/so
# add_library(zc_test_singleton STATIC ./)
aux_source_directory(./ SUB_DIR_SOURCE_SRCS)
#.a/so
add_library(zc_test_singleton STATIC ${SUB_DIR_SOURCE_SRCS})
target_include_directories(zc_test_singleton PRIVATE
${UTILSXX_INC}
${UTILS_INC}
)

#install(TARGETS zc_test_singleton DESTINATION ${ZC_LIB_INSTALL})
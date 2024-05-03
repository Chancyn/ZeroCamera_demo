cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

#.a/so
# add_library(zc_test_nngxx STATIC ${DIR_SOURCE_SRCS})
add_library(zc_test_nngxx SHARED ${DIR_SOURCE_SRCS})
#add_library(zc_test_nngxx STATIC ${DIR_SOURCE_SRCS})

#头文件包含
target_include_directories(zc_test_nngxx PRIVATE
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
${NNGXX_INC}
${NNG_INC}
)

# #连接库
# target_link_libraries(zc_test_nngxx PUBLIC
# plat_basic
# bsw_basic
# utilsxx
# utils
# )

install(TARGETS zc_test_nngxx DESTINATION ${ZC_LIB_INSTALL})
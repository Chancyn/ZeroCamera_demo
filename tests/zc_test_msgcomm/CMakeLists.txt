cmake_minimum_required(VERSION 3.22)
# aux_source_directory(./ DIR_SOURCE_SRCS)
aux_source_directory(./ DIR_SOURCE_SRCS)
foreach(SOURCE_FILE ${DIR_SOURCE_SRCS})
	get_filename_component(filename ${SOURCE_FILE} NAME)
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${filename}\""
    )
endforeach()

#.a/so
# add_library(zc_test_msgcomm STATIC ${DIR_SOURCE_SRCS})
add_library(zc_test_msgcomm SHARED ${DIR_SOURCE_SRCS})
#add_library(zc_test_msgcomm STATIC ${DIR_SOURCE_SRCS})

#头文件包含
target_include_directories(zc_test_msgcomm PRIVATE
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
${MSGCOMM_INC}
)

# #连接库
# target_link_libraries(zc_test_msgcomm PUBLIC
# plat_basic
# bsw_basic
# utilsxx
# utils
# )

install(TARGETS zc_test_msgcomm DESTINATION ${ZC_LIB_INSTALL})
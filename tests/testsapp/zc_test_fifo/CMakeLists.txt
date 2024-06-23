cmake_minimum_required(VERSION 3.13)
aux_source_directory(./ SUB_DIR_SOURCE_SRCS)

foreach(SOURCE_FILE ${SUB_DIR_SOURCE_SRCS})
	get_filename_component(filename ${SOURCE_FILE} NAME)
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${filename}\""
    )
endforeach()

#.a/so
#add_library(zc_test_fifo SHARED ${SUB_DIR_SOURCE_SRCS})
add_library(zc_test_fifo STATIC ${SUB_DIR_SOURCE_SRCS})


#头文件包含
target_include_directories(zc_test_fifo PRIVATE
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
)

# #连接库
# target_link_libraries(zc_test_fifo PUBLIC
# plat_basic
# bsw_basic
# utilsxx
# utils
# )

install(TARGETS zc_test_fifo DESTINATION ${ZC_LIB_INSTALL})
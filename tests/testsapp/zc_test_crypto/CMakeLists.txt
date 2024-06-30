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
add_library(zc_test_crypto STATIC ${SUB_DIR_SOURCE_SRCS})

#头文件包含
target_include_directories(zc_test_crypto PRIVATE
)

install(TARGETS zc_test_crypto DESTINATION ${ZC_LIB_INSTALL})
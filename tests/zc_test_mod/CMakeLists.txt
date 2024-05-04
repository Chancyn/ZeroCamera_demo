cmake_minimum_required(VERSION 3.22)
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
# add_library(zc_test_mod STATIC ${SUB_DIR_SOURCE_SRCS})
add_library(zc_test_mod SHARED ${SUB_DIR_SOURCE_SRCS})
#add_library(zc_test_mod STATIC ${SUB_DIR_SOURCE_SRCS})



#头文件包含
target_include_directories(zc_test_mod PRIVATE
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
)

# #连接库
# target_link_libraries(zc_test_mod PUBLIC
# plat_basic
# bsw_basic
# utilsxx
# utils
# )

install(TARGETS zc_test_mod DESTINATION ${ZC_LIB_INSTALL})
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
# add_library(zc_test_utilsxx STATIC ${SUB_DIR_SOURCE_SRCS})
add_library(zc_test_utilsxx STATIC ${SUB_DIR_SOURCE_SRCS})
#add_library(zc_test_utilsxx STATIC ${SUB_DIR_SOURCE_SRCS})

#头文件包含
target_include_directories(zc_test_utilsxx PRIVATE
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
)

# #连接库
target_link_libraries(zc_test_utilsxx PUBLIC
zc_test_observer
zc_test_singleton
zc_test_thread
zc_test_timer
zc_test_epoll
plat_basic
bsw_basic
utilsxx
utils
)


add_subdirectory(zc_test_observer)
add_subdirectory(zc_test_singleton)
add_subdirectory(zc_test_thread)
add_subdirectory(zc_test_timer)
add_subdirectory(zc_test_epoll)
#add_subdirectory(zc_test_utilsxx_main)

install(TARGETS zc_test_utilsxx DESTINATION ${ZC_LIB_INSTALL})
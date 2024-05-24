cmake_minimum_required(VERSION 3.22)
#aux_source_directory(./ DIR_SOURCE_SRCS)
add_executable(zc_tests zc_test_main.cpp)
foreach(SOURCE_FILE zc_test_main.cpp)
	get_filename_component(filename ${SOURCE_FILE} NAME)
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${filename}\""
    )
endforeach()

#add_executable(zc_tests zc_test_main.cpp)
add_subdirectory(zc_test_utilsxx)
add_subdirectory(zc_test_msgcomm)
add_subdirectory(zc_test_mod)
add_subdirectory(zc_test_fifo)
#头文件包含
#头文件包含
target_include_directories(zc_tests PRIVATE
zc_test_utilsxx
zc_test_mod
zc_test_fifo
zc_test_msgcomm
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
${MSGCOMM_INC}
)

target_link_directories(zc_tests PRIVATE
${NNG_LIBPATH}
)

#连接库
target_link_libraries(zc_tests PRIVATE
zc_test_mod
zc_test_fifo
mod_rtsp
mod_sys
mod_base
zc_test_msgcomm
msgcomm
zc_test_utilsxx
zcfifo
zc_log
# plat_basic
# bsw_basic
# utilsxx
# utils
pthread
nng
)

enable_testing()
add_test(NAME zc_tests COMMAND Test)

#执行文件的安装位置
install(TARGETS zc_tests RUNTIME DESTINATION ${ZC_BIN_INSTALL})
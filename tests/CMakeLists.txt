cmake_minimum_required(VERSION 3.22)

add_executable(zc_tests zc_test_main.cpp)
#add_executable(zc_test zc_test_main.cpp)
add_subdirectory(zc_test_utilsxx)

#头文件包含
#头文件包含
target_include_directories(zc_tests PRIVATE
zc_test_utilsxx
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
)

# target_link_directories(zc_tests PRIVATE
# ${SPDLOG_LIBPATH}
# )

#连接库
target_link_libraries(zc_tests PRIVATE
zc_test_utilsxx
zc_log
# plat_basic
# bsw_basic
# utilsxx
# utils
# pthread
)

enable_testing()
add_test(NAME zc_tests COMMAND Test)

#执行文件的安装位置
install(TARGETS zc_tests RUNTIME DESTINATION ${ZC_BIN_INSTALL})
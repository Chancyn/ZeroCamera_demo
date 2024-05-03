cmake_minimum_required(VERSION 3.22)

add_executable(zc_tests zc_test_main.cpp)
#add_executable(zc_test zc_test_main.cpp)
add_subdirectory(zc_test_utilsxx)
add_subdirectory(zc_test_nngxx)
add_subdirectory(zc_test_mod)
#头文件包含
#头文件包含
target_include_directories(zc_tests PRIVATE
zc_test_utilsxx
zc_test_mod
zc_test_nngxx
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
${NNGXX_INC}
${NNG_INC}
)

target_link_directories(zc_tests PRIVATE
${NNG_LIBPATH}
)

#连接库
target_link_libraries(zc_tests PRIVATE
zc_test_nngxx
nngxx
nng
zc_test_utilsxx
zc_test_mod
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
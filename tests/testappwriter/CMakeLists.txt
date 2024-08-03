cmake_minimum_required(VERSION 3.13)
#aux_source_directory(./ DIR_SOURCE_SRCS)
add_executable(zc_testwriter zc_testwriter_main.cpp ZcTestWriterMan.cpp)
foreach(SOURCE_FILE zc_testwriter_main.cpp)
	get_filename_component(filename ${SOURCE_FILE} NAME)
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${filename}\""
    )
endforeach()

#add_executable(zc_tests zc_test_main.cpp)
#头文件包含
#头文件包含
target_include_directories(zc_testwriter PRIVATE
${ZCRTSP_INC}
${ZCFMP4_INC}
${ZCWRITER_INC}
${MEDIAAVCODEC_INC}
${MEDIAAVBSF_INC}
${MEDIAMPEG_INC}
${MEDIAMOV_INC}
${MEDIAFLV_INC}
${MEDIAMKV_INC}
${MEDIARTP_INC}
${MEDIAAIO_INC}
${MEDIAHTTP_INC}
${MEDIARTSP_INC2}
${MEDIARTSP_INC}
${MEDIASDK_INC}
${PLAT_BASIC_INC}
${UTILSXX_INC}
${UTILS_INC}
)

target_link_directories(zc_testwriter PRIVATE
${MEDIARTSP_LIBPATH}
${MEDIAAVBSF_LIBPATH}
${MEDIAAVCODEC_LIBPATH}
${MEDIAMOV_LIBPATH}
${MEDIAMPEG_LIBPATH}
${MEDIAFLV_LIBPATH}
${MEDIAMKV_LIBPATH}
${MEDIARTP_LIBPATH}
${MEDIAHTTP_LIBPATH}
${MEDIAAIO_LIBPATH}
${MEDIASDK_LIBPATH}

${NNG_LIBPATH}
)

#连接库
target_link_libraries(zc_testwriter PRIVATE
zcrtsp
zcwriter
zcfmp4
${MEDIARTSPMEDIA}
zch26x
${MEDIAAIO}
${MEDIARTSPAIO}
${MEDIAFILEBUF}
${MEDIARTSP}
${MEDIAAVBSF}
${MEDIAAVCODEC}
${MEDIAMPEG}
${MEDIAMOV}
${MEDIARTP}
${MEDIAFLV}
${MEDIAMKV}
${MEDIAHTTP}
${MEDIASDK}
mod_base
msgcomm
zcfifo
zccrypto
zc_log
# plat_basic
# bsw_basic
# utilsxx
# utils
nng
pthread
)

#asan
if(WITH_ASAN)
target_link_libraries(zc_testwriter PRIVATE
asan.a
)
endif()

enable_testing()
add_test(NAME zc_testwriter COMMAND Test)

#执行文件的安装位置
install(TARGETS zc_testwriter RUNTIME DESTINATION ${ZC_BIN_INSTALL})
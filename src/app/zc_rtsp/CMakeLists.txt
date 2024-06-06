cmake_minimum_required(VERSION 3.13)
aux_source_directory(./ DIR_SOURCE_SRCS)

foreach(SOURCE_FILE ${DIR_SOURCE_SRCS})
	get_filename_component(filename ${SOURCE_FILE} NAME)
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${filename}\""
    )
endforeach()

add_executable(zc_rtsp ${DIR_SOURCE_SRCS})

target_include_directories(zc_rtsp PRIVATE
${ZCRTSP_INC}
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
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
)

target_link_directories(zc_rtsp PRIVATE
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

target_link_libraries(zc_rtsp PUBLIC
zcrtsp
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
mod_rtsp
mod_base
msgcomm
utilsxx
utils
zcfifo
plat_basic
bsw_basic
nng
zc_log
)

#asan
if(WITH_ASAN)
target_link_libraries(zc_rtsp PRIVATE
asan.a
)
endif()

#install(TARGETS zc_rtsp RUNTIME DESTINATION ${ZC_BIN_INSTALL})
install(TARGETS zc_rtsp DESTINATION ${ZC_BIN_INSTALL})
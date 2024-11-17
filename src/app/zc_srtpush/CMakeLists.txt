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

add_executable(zc_srtpush ${DIR_SOURCE_SRCS})

target_include_directories(zc_srtpush PRIVATE
${ZCSRT_INC}
${ZCMPEG_INC}
${MEDIAAVCODEC_INC}
${MEDIAAVBSF_INC}
${MEDIAMPEG_INC}
${MEDIARTP_INC}
${MEDIAAIO_INC}
${MEDIASDK_INC}
${PLAT_BASIC_INC}
)

target_link_directories(zc_srtpush PRIVATE
${MEDIAAVBSF_LIBPATH}
${MEDIAAVCODEC_LIBPATH}
${MEDIAMPEG_LIBPATH}
${MEDIARTP_LIBPATH}
${MEDIAAIO_LIBPATH}
${MEDIASDK_LIBPATH}
${SRT_LIBPATH}
${NNG_LIBPATH}
)

target_link_libraries(zc_srtpush PUBLIC
zcsrt
zcmpeg
${MEDIARTSPMEDIA}
zch26x
${MEDIAAIO}
${MEDIAAVBSF}
${MEDIAAVCODEC}
${MEDIAMPEG}
${MEDIARTP}
${MEDIASDK}
mod_rtsp
mod_base
msgcomm
utilsxx
utils
zcfifo
plat_basic
bsw_basic
zccrypto
nng
zc_log
)

#asan
if(WITH_ASAN)
target_link_libraries(zc_srtpush PRIVATE
asan.a
)
endif()

#install(TARGETS zc_srtpush RUNTIME DESTINATION ${ZC_BIN_INSTALL})
install(TARGETS zc_srtpush DESTINATION ${ZC_BIN_INSTALL})
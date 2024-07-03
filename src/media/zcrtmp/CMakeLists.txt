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

add_library(zcrtmp STATIC ${DIR_SOURCE_SRCS})

target_include_directories(zcrtmp PRIVATE
${MEDIAAVCODEC_INC}
${MEDIAAVBSF_INC}
${MEDIAMPEG_INC}
${MEDIAMOV_INC}
${MEDIAFLV_INC}
${MEDIAMKV_INC}
${MEDIARTP_INC}
${MEDIAAIO_INC}
${MEDIAHTTP_INC}
${MEDIARTMP_INC}
${MEDIASDK_INC}
${PLAT_BASIC_INC}
)

target_link_directories(zcrtmp PRIVATE
${MEDIARTMP_LIBPATH}
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

target_link_libraries(zcrtmp PUBLIC
${MEDIAFILEBUF}
${MEDIARTMP}
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

install(TARGETS zcrtmp DESTINATION ${ZC_LIB_INSTALL})
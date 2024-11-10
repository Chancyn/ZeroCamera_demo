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

add_library(zcsrt STATIC ${DIR_SOURCE_SRCS})

target_include_directories(zcsrt PRIVATE
${SRT_INC}
${ZCMPEG_INC}
${MEDIAAVCODEC_INC}
${MEDIAAVBSF_INC}
${PLAT_BASIC_INC}
)

target_link_libraries(zcsrt PUBLIC
srt
zcmpeg
${MEDIAAVBSF}
${MEDIAAVCODEC}
${MEDIAMPEG}
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

install(TARGETS zcsrt DESTINATION ${ZC_LIB_INSTALL})
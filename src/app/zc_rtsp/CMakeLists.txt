cmake_minimum_required(VERSION 3.22)
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
${MEDIAHTTP_LIBPATH}
${MEDIAAIO_LIBPATH}
${MEDIASDK_LIBPATH}
)

target_link_libraries(zc_rtsp PUBLIC
${MEDIARTSPAIO}
${MEDIARTSP}
${MEDIAHTTP}
${MEDIAAIO}
${MEDIASDK}
mod_rtsp
mod_base
msgcomm
utilsxx
utils
plat_basic
bsw_basic
zc_log
)

add_subdirectory(aio)

#install(TARGETS zc_rtsp RUNTIME DESTINATION ${ZC_BIN_INSTALL})
install(TARGETS zc_rtsp DESTINATION ${ZC_BIN_INSTALL})
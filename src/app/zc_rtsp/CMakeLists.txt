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
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
)

# target_link_directories(zc_rtsp PRIVATE
# ${SPDLOG_LIBPATH}
# )

target_link_libraries(zc_rtsp PUBLIC
mod_rtsp
mod_base
msgcomm
plat_basic
bsw_basic
zc_log
)

#install(TARGETS zc_rtsp RUNTIME DESTINATION ${ZC_BIN_INSTALL})
install(TARGETS zc_rtsp DESTINATION ${ZC_BIN_INSTALL})
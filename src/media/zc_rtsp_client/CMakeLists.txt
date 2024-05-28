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
add_library(zc_rtsp_client STATIC ${DIR_SOURCE_SRCS})
#add_library(bsw_basic SHARED ${DIR_SOURCE_SRCS})

install(TARGETS zc_rtsp_client DESTINATION ${ZC_LIB_INSTALL})

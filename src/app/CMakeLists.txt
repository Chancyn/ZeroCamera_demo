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

#add_definitions(-D__ERROR__=00*10000000+__LINE__*1000)

add_subdirectory(zc_sys)
add_subdirectory(zc_codec)
add_subdirectory(zc_rtsp)
add_subdirectory(zc_rtspcli)
add_subdirectory(zc_rtsppushcli)
add_subdirectory(zc_rtsppushs)
add_subdirectory(zc_rtmppush)
add_subdirectory(zc_rtmpsvr)
add_subdirectory(zc_webs)
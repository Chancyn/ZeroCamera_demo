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

add_subdirectory(zcrtsp)
add_subdirectory(zch26x)
add_subdirectory(zccodec)
add_subdirectory(zcflv)
add_subdirectory(zcrtmp)
add_subdirectory(zcfmp4)
add_subdirectory(zcwriter)
add_subdirectory(zcmpeg)
add_subdirectory(zcsrt)
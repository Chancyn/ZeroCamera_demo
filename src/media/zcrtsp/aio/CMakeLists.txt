cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ SUB_DIR_SOURCE_SRCS)

foreach(SOURCE_FILE ${SUB_DIR_SOURCE_SRCS})
	get_filename_component(filename ${SOURCE_FILE} NAME)
    set_source_files_properties(
        ${SOURCE_FILE}
        PROPERTIES
        COMPILE_DEFINITIONS "__FILENAME__=\"${filename}\""
    )
endforeach()

#.a/so
add_library(${MEDIARTSPAIO} STATIC ${SUB_DIR_SOURCE_SRCS})
#add_library(${MEDIARTSPAIO} SHARED ${SUB_DIR_SOURCE_SRCS})

target_include_directories(${MEDIARTSPAIO} PRIVATE
${MEDIARTSP_INC}
${MEDIAAIO_INC}
${MEDIASDK_INC}
${MEDIAHTTP_INC}
)

#install(TARGETS ${MEDIARTSPAIO} RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS ${MEDIARTSPAIO} DESTINATION ${ZC_LIB_INSTALL})
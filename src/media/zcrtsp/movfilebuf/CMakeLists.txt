cmake_minimum_required(VERSION 3.13)
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
add_library(${MEDIAFILEBUF} STATIC ${SUB_DIR_SOURCE_SRCS})
#add_library(${MEDIAFILEBUF} SHARED ${SUB_DIR_SOURCE_SRCS})

target_include_directories(${MEDIAFILEBUF} PRIVATE
${MEDIAMOV_INC}
${MEDIASDK_INC}
)

#install(TARGETS ${MEDIAFILEBUF} RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS ${MEDIAFILEBUF} DESTINATION ${ZC_LIB_INSTALL})
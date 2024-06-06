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

#.a/so
add_library(msgcomm SHARED ${DIR_SOURCE_SRCS})
#add_library(msgcomm SHARED ${DIR_SOURCE_SRCS})

#message("build log msgcomm inc:${NNG_INC}")

target_include_directories(msgcomm PUBLIC
${NNG_INC}
)

#target_link_directories(msgcomm PRIVATE ${NNG_LIBPATH})
#target_link_libraries(msgcomm PRIVATE nng)

#install(TARGETS msgcomm RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS msgcomm DESTINATION ${ZC_LIB_INSTALL})
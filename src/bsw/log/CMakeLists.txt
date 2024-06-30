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
add_library(zc_log SHARED ${DIR_SOURCE_SRCS})
#add_library(zc_log SHARED ${DIR_SOURCE_SRCS})

message("build log zc_log inc:${SPDLOG_INC}")
target_include_directories(zc_log PUBLIC ${SPDLOG_INC} )

#target_link_directories(zc_log PRIVATE ${SPDLOG_LIBPATH})
#target_link_libraries(zc_log PRIVATE spdlog)

#target_link_directories(zc_log PUBLIC ${SPDLOG_LIBPATH})
#install(TARGETS zc_log RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS zc_log DESTINATION ${ZC_LIB_INSTALL})
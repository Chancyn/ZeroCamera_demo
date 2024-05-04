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

#.a/so
add_library(bsw_basic STATIC ${DIR_SOURCE_SRCS})
#add_library(bsw_basic SHARED ${DIR_SOURCE_SRCS})

#target_include_directories(bsw_basic PRIVATE ${ZC_LOG_INC})

#install(TARGETS bsw_basic RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS bsw_basic DESTINATION ${ZC_LIB_INSTALL})
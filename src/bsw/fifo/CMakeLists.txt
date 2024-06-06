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
add_library(zcfifo STATIC ${DIR_SOURCE_SRCS})
#add_library(zcfifo SHARED ${DIR_SOURCE_SRCS})

#target_include_directories(zcfifo PRIVATE ${ZC_LOG_INC})

#install(TARGETS zcfifo RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS zcfifo DESTINATION ${ZC_LIB_INSTALL})
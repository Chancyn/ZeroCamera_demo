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

aux_source_directory(./ DIR_SOURCE_SRCS)
#.a/so
add_library(zc_test_timer STATIC ${DIR_SOURCE_SRCS})
#add_library(zc_test_timer STATIC ./)
target_include_directories(zc_test_timer PRIVATE
${UTILSXX_INC}
${UTILS_INC}
)

#install(TARGETS zc_test_timer DESTINATION ${ZC_LIB_INSTALL})
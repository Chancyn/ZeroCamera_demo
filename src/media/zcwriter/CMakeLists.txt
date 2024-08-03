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

add_library(zcwriter STATIC ${DIR_SOURCE_SRCS})

target_include_directories(zcwriter PRIVATE
${MEDIAMOV_INC}
${MEDIAFLV_INC}
${PLAT_BASIC_INC}
)

target_link_directories(zcwriter PRIVATE
${MEDIAMOV_LIBPATH}
${MEDIAFLV_LIBPATH}
)

target_link_libraries(zcwriter PUBLIC
${MEDIAMOV}
${MEDIAFLV}

utilsxx
utils
zcfifo
plat_basic
bsw_basic
nng
zc_log
)

install(TARGETS zcwriter DESTINATION ${ZC_LIB_INSTALL})
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

add_executable(zc_sys ${DIR_SOURCE_SRCS})

target_include_directories(zc_sys PRIVATE
# ${NNG_INC}
${BSW_BASIC_INC}
${PLAT_BASIC_INC}
${MSGCOMM_INC}
)

target_link_directories(zc_sys PRIVATE
${NNG_LIBPATH}
)

target_link_libraries(zc_sys PRIVATE
mod_sys
mod_base
msgcomm
utilsxx
utils
plat_basic
bsw_basic
zc_log
nng
#pthread
)

#asan
if(WITH_ASAN)
target_link_libraries(zc_sys PRIVATE
asan.a
)
endif()

install(TARGETS zc_sys RUNTIME DESTINATION ${ZC_BIN_INSTALL})
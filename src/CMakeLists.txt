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

#thirdparty
# set(SPDLOG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/spdlog/include)
# set(SPDLOG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/spdlog/lib)
# set(NNG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/include)
# set(NNG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/lib)

# set(BSW_BASIC_INC ${CMAKE_SOURCE_DIR}/src/bsw/basic)
# set(PLAT_BASIC_INC ${CMAKE_SOURCE_DIR}/src/plat/basic)
# set(MSGCOMM_INC ${CMAKE_SOURCE_DIR}/src/bsw/msgcomm)
# set(UTILSXX_INC ${CMAKE_SOURCE_DIR}/src/bsw/utilsxx)

message("build with src")

add_subdirectory(bsw)
add_subdirectory(plat)
add_subdirectory(mod)
add_subdirectory(media)
add_subdirectory(app)


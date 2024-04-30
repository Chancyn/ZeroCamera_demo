cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

# set(NNG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/include)
# set(NNG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/lib)

#.a/so
add_library(zc_nng SHARED ${DIR_SOURCE_SRCS})
#add_library(zc_nng SHARED ${DIR_SOURCE_SRCS})

#message("build log zc_nng inc:${NNG_INC}")

target_include_directories(zc_nng PUBLIC
${NNG_INC}
)

target_link_directories(zc_nng PRIVATE ${NNG_LIBPATH})
target_link_libraries(zc_nng PRIVATE nng)

#install(TARGETS zc_nng RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS zc_nng DESTINATION ${ZC_LIB_INSTALL})
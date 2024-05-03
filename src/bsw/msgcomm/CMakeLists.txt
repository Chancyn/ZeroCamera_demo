cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

# set(NNG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/include)
# set(NNG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/lib)

#.a/so
add_library(msgcomm SHARED ${DIR_SOURCE_SRCS})
#add_library(msgcomm SHARED ${DIR_SOURCE_SRCS})

#message("build log msgcomm inc:${NNG_INC}")

target_include_directories(msgcomm PUBLIC
${NNG_INC}
)

target_link_directories(msgcomm PRIVATE ${NNG_LIBPATH})
target_link_libraries(msgcomm PRIVATE nng)

#install(TARGETS msgcomm RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS msgcomm DESTINATION ${ZC_LIB_INSTALL})
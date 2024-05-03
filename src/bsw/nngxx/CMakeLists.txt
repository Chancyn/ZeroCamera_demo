cmake_minimum_required(VERSION 3.22)
aux_source_directory(./ DIR_SOURCE_SRCS)

# set(NNG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/include)
# set(NNG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/lib)

#.a/so
add_library(nngxx SHARED ${DIR_SOURCE_SRCS})
#add_library(nngxx SHARED ${DIR_SOURCE_SRCS})

#message("build log nngxx inc:${NNG_INC}")

target_include_directories(nngxx PUBLIC
${NNG_INC}
)

target_link_directories(nngxx PRIVATE ${NNG_LIBPATH})
target_link_libraries(nngxx PRIVATE nng)

#install(TARGETS nngxx RUNTIME DESTINATION ${ZC_LIB_INSTALL})
install(TARGETS nngxx DESTINATION ${ZC_LIB_INSTALL})
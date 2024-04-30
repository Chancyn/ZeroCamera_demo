cmake_minimum_required(VERSION 3.22)

#thirdparty
# set(SPDLOG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/spdlog/include)
# set(SPDLOG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/spdlog/lib)
# set(NNG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/include)
# set(NNG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/lib)

# set(BSW_BASIC_INC ${CMAKE_SOURCE_DIR}/src/bsw/basic)
# set(PLAT_BASIC_INC ${CMAKE_SOURCE_DIR}/src/plat/basic)
# set(ZC_NNG_INC ${CMAKE_SOURCE_DIR}/src/bsw/zc_nng)
# set(UTILSXX_INC ${CMAKE_SOURCE_DIR}/src/bsw/utilsxx)

message("build with src")

add_subdirectory(bsw)
add_subdirectory(plat)
add_subdirectory(mod)
add_subdirectory(app)


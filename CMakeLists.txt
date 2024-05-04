cmake_minimum_required(VERSION 3.0.0)
project(zc_test VERSION 0.1.0 LANGUAGES C CXX)

#set the project name
message("build $ENV{soc}")
set(ZC_SOC $ENV{soc})

#install
set(ZC_BIN_INSTALL ${PROJECT_SOURCE_DIR}/output/${ZC_SOC}/bin)
set(ZC_LIB_INSTALL ${PROJECT_SOURCE_DIR}/output/${ZC_SOC}/lib)

set(CMAKE_SYSTEM_NAME Linux)    #设置目标系统名字

#soc empty
set (CFLAGS_SOC )
set (CXXFLAGS_SOC )

# set platform
if("${ZC_SOC}x" STREQUAL "hi3519dv500x" )
message("build hi3519dv500")
set(COMPILER_PREFIX "aarch64-v01c01-linux-gnu-")
set(CMAKE_SYSTEM_PROCESSOR arm)
set (CFLAGS_SOC -mcpu=cortex-a55)
set (CXXFLAGS_SOC -mcpu=cortex-a55)
else()
message("build ${ZC_SOC}")
endif()

#set(filename "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/")
#add_definitions(-D__FILENAME__="${filename}")

set (CMAKE_C_COMPILER ${COMPILER_PREFIX}gcc)
set (CMAKE_CXX_COMPILER ${COMPILER_PREFIX}g++)

set(CMAKE_C_FLAGS
    "${CFLAGS_SOC} -Wall -std=gnu11 -fPIC"
)

#"${CXXFLAGS_SOC} -Wall -std=c++11 -Werror" -std=c++11
set(CMAKE_CXX_FLAGS
    "${CXXFLAGS_SOC} -Wall -std=gnu++11 -fPIC"
)

#build cmd debug/release
if(NOT CMAKE_BUILD_TYPE)
message("build not define CMAKE_BUILD_TYPE")
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")
message("build default " ${CMAKE_BUILD_TYPE})
else()
message("build with " ${CMAKE_BUILD_TYPE})
endif()

if("${CMAKE_BUILD_TYPE}x" STREQUAL "Debugx")
       #SET(CMAKE_ZC_DEBUG 1)
       message("build with Debug")
       add_definitions(-DZC_DEBUG)
       #add_definitions(-DSPDLOG_ACTIVE_LEVEL=0)
else()
       #SET(CMAKE_ZC_DEBUG 0)
       message("build with Release")
endif()


set(SPDLOG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/spdlog/include)
set(SPDLOG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/spdlog/lib)
set(NNG_INC ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/include)
set(NNG_LIBPATH ${PROJECT_SOURCE_DIR}/thirdparty/install/${ZC_SOC}/nng/lib)

set(BSW_BASIC_INC ${CMAKE_SOURCE_DIR}/src/bsw/basic)
set(PLAT_BASIC_INC ${CMAKE_SOURCE_DIR}/src/plat/basic)
set(MSGCOMM_INC ${CMAKE_SOURCE_DIR}/src/bsw/msgcomm)
set(UTILSXX_INC ${CMAKE_SOURCE_DIR}/src/bsw/utilsxx)

include_directories(include
include/zc_log
include/utilsxx
include/utils
include/msgcomm
include/mod
)

add_subdirectory(src)
add_subdirectory(tests)

#add_subdirectory(tests)
#option build if or not
#option(WITH_BSW "bsw option"     ON)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


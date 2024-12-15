#! /bin/bash

basepath=$(cd `dirname $01`; pwd)

srcpkg=srt-1.5.4.tar.gz
srcpkgbasename=srt-1.5.4
srcname=srt
srcdir=$basepath/srt
installabsdir=$basepath/../../install/ss928v100/${srcname}
installopenssl=$basepath/../../install/ss928v100/openssl
[ -d $installabsdir ] || mkdir -p $installabsdir
installdir=$(cd $installabsdir; pwd)

COMPILER_PREFIX=aarch64-mix210-linux-
export CC=${COMPILER_PREFIX}gcc
export CXX=${COMPILER_PREFIX}g++

#export CXXFLAGS='-mcpu=cortex-a53 -mfloat-abi=softfp -mfpu=neon-vfpv4 -mno-unaligned-access -fno-aggressive-loop-optimizations -fPIC -ffunction-sections'
#export CFLAGS='-mcpu=cortex-a55'
#export CXXFLAGS='-mcpu=cortex-a55'
#export CFLAGS='-mcpu=cortex-a55 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE'
#export CXXFLAGS='-mcpu=cortex-a55 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE'

################################################################################
function check_builddir()
{
    if [ ! -d $srcdir ]; then
        #echo "error ${srcdir} not exist"
        if [ -e $srcpkg ]; then
        echo "error ${srcdir} not exist,unpack package ${srcpkg}"
        tar -zxvf ${srcpkg}
        mv ${srcpkgbasename} ${srcdir}
        else
        echo "error ${srcpkg} not exist"
        exit -1;
        fi
    fi
}

function check_installdir()
{
    echo "check installdir ${installdir}"
    [ -d $installdir ] || mkdir -p $installdir
}

function build_cmake_shared()
{
    pushd ${srcdir}
    [ -d ${srcdir}/build ] && rm ${srcdir}/build -rf
    mkdir ${srcdir}/build && cd ${srcdir}/build
    cmake .. -DUSE_OPENSSL_PC=OFF -DOPENSSL_ROOT_DIR=${installopenssl} -DCMAKE_INSTALL_PREFIX=$installdir -DBUILD_SHARED_LIBS=true
    #cmake .. -DCMAKE_C_FLAGS= -DCMAKE_INSTALL_PREFIX=$installdir -DBUILD_SHARED_LIBS=true
    make clean
    make -j 16
    rm -rf $installdir
    make install
    popd
}

#default static lib
function build_cmake()
{
    pushd ${srcdir}
    [ -d ${srcdir}/build ] && rm ${srcdir}/build -rf
    mkdir ${srcdir}/build && cd ${srcdir}/build
    #cmake ..  -DCMAKE_INSTALL_PREFIX=$installdir -DBUILD_STATIC_LIBS=true
    cmake .. -DUSE_OPENSSL_PC=OFF -DOPENSSL_ROOT_DIR=${installopenssl} -DCMAKE_INSTALL_PREFIX=$installdir
    make clean
    make -j16
    rm -rf $installdir
    make install
    popd
}

check_builddir
check_installdir
build_cmake
#build_cmake_shared

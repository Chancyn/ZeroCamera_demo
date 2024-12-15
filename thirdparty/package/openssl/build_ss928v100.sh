#! /bin/bash

basepath=$(cd `dirname $01`; pwd)

srcpkg=openssl-1.1.1w.tar.gz
srcpkgbasename=openssl-1.1.1w
# srcpkg=openssl-3.4.0.tar.gz
# srcpkgbasename=openssl-3.4.0
srcname=openssl
srcdir=$basepath/openssl
installabsdir=$basepath/../../install/ss928v100/${srcname}
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

function build_make()
{
    pushd $srcdir
    [ -e Makefile ] && rm Makefile

    ./config ARCH=arm64 --cross-compile-prefix=${COMPILER_PREFIX} no-asm shared no-async --prefix=$installdir
    sed -i 's/^CNF_CFLAGS=-pthread -m64/CNF_CFLAGS=-pthread/' Makefile
    sed -i 's/^CNF_CXXFLAGS=-std=c++11 -pthread -m64/CNF_CXXFLAGS=-std=c++11 -pthread/' Makefile

    make clean
    make -j16
    make install
    popd
}

function check_installdir()
{
    echo "check installdir ${installdir}"
    [ -d $installdir ] || mkdir -p $installdir
}

check_builddir
check_installdir
build_make

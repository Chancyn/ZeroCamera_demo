#! /bin/bash

basepath=$(cd `dirname $01`; pwd)

srcpkg=libv4l2cpp-master.tar.gz
srcpkgbasename=libv4l2cpp-master
srcname=v4l2cpp
srcdir=$basepath/v4l2cpp

installabsdir=$basepath/../../install/x86_64/${srcname}

[ -d $installabsdir ] || mkdir -p $installabsdir
installdir=$(cd $installabsdir; pwd)

COMPILER_PREFIX=
export CC=${COMPILER_PREFIX}gcc
export CXX=${COMPILER_PREFIX}g++

#debug
#export CFLAGS='-DSPDLOG_ACTIVE_LEVEL=0'
#export CXXFLAGS='-DSPDLOG_ACTIVE_LEVEL=0'

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

    ./configure --host=arm-linux --prefix=$installdir

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

function build_cmake_shared()
{
    pushd ${srcdir}
    [ -d ${srcdir}/build ] && rm ${srcdir}/build -rf
    mkdir ${srcdir}/build && cd ${srcdir}/build
    cmake ..  -DCMAKE_INSTALL_PREFIX=$installdir -DBUILD_SHARED_LIBS=true
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
    cmake ..  -DCMAKE_INSTALL_PREFIX=$installdir
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
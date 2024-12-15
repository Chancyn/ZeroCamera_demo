#! /bin/bash

basepath=$(cd `dirname $01`; pwd)

srcpkg=openssl-1.1.1w.tar.gz
srcpkgbasename=openssl-1.1.1w
srcname=openssl
srcdir=$basepath/openssl

installabsdir=$basepath/../../install/x64/${srcname}

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

    ./config --prefix=$installdir

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

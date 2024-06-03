#! /bin/bash

basepath=$(cd `dirname $01`; pwd)

#sdk
srcpkg=sdk.tar.gz
srcpkgbasename=sdk
srcname=sdk
srcdir=$basepath/${srcname}
installabsdir=$basepath/../../install/x64/media_server/${srcname}
[ -d $installabsdir ] || mkdir -p $installabsdir
installdir=$(cd $installabsdir; pwd)

#debug/release
RELEASE=0
if [ x$RELEASE == x1 ]; then
	BUILD=release
else
	BUILD=debug
fi

#platform=aarch64-v01c01-linux-gnu
platform=
if [ $platform ]; then
builddir=${BUILD}.${platform}
buildargs="PLATFORM=${platform} RELEASE=${RELEASE}"
COMPILER_PREFIX=${platform}-
else
ARCHBITS=64
OSID=$(awk -F'=' '/^ID=/ {print $2}' /etc/os-release | tr -d '"')
OSVERSIONID=$(awk -F'=' '/^VERSION_ID=/ {print $2"-"}' /etc/os-release | tr -d '"')
builddir=${BUILD}.${OSID}${OSVERSIONID}linux${ARCHBITS}
buildargs="RELEASE=${RELEASE}"
COMPILER_PREFIX=
fi

export CC=${COMPILER_PREFIX}gcc
export CXX=${COMPILER_PREFIX}g++

echo "installdir:${installdir}"
echo "builddir:${builddir}"
echo "buildargs:${buildargs}"

#debug
export CXXFLAGS=-g
export CFLAGS=-g
#export CXXFLAGS='-mcpu=cortex-a53 -mfloat-abi=softfp -mfpu=neon-vfpv4 -mno-unaligned-access -fno-aggressive-loop-optimizations -fPIC -ffunction-sections'

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
    #
    #make PLATFORM=aarch64-v01c01-linux-gnu RELEASE=0
    make ${buildargs} clean
    make ${buildargs} -j 16
    make_exit_code=$?
    if [ $make_exit_code -eq 0 ]; then
    echo "make succeeded"
    else
        echo "make failed with exit code $make_exit_code"
        exit $make_exit_code
    fi
    # make install
    popd
}

function check_installdir()
{
    echo "check installdir ${installdir}"
    if [ -d $installdir ] ;then
        echo "clear installdir ${installdir}"
        rm $installdir -rf
    else
        mkdir -p $installdir
    fi
}

function build_installdir()
{
    echo "builddir ${builddir} ${installdir}"
    check_installdir
    #sdk
    echo "copy libsdk ${srcname}/libsdk/${builddir}"
    mkdir -p $installdir/libsdk/lib
    cp ${srcname}/libsdk/${builddir}/lib* $installdir/libsdk/lib/ -rf
    cp ${srcname}/include $installdir -rf

    #aio
    echo "copy libaio ${srcname}/libaio/${builddir}"
    mkdir -p $installdir/libaio/lib
    cp ${srcname}/libaio/${builddir}/lib* $installdir/libaio/lib/ -rf
    cp ${srcname}/libaio/include $installdir/libaio -rf

    #http
    echo "copy libhttp ${srcname}/libhttp/${builddir}"
    mkdir -p $installdir/libhttp/lib
    cp ${srcname}/libhttp/${builddir}/lib* $installdir/libhttp/lib/ -rf
    cp ${srcname}/libhttp/include $installdir/libhttp/ -rf

    #ice
    echo "copy libice ${srcname}/libice/${builddir}"
    mkdir -p $installdir/libice/lib
    cp ${srcname}/libice/${builddir}/lib* $installdir/libice/lib/ -rf
    cp ${srcname}/libice/include $installdir/libice/ -rf

}


check_builddir
build_make
build_installdir

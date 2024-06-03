#! /bin/bash

basepath=$(cd `dirname $01`; pwd)

#sdk
srcpkg=media-server.tar.gz
srcpkgbasename=media-server
srcname=media-server
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
    #libdash
    echo "copy libdash ${srcname}/libdash/${builddir}"
    mkdir -p $installdir/libdash/lib
    cp ${srcname}/libdash/${builddir}/lib* $installdir/libdash/lib/ -rf
    cp ${srcname}/libdash/include $installdir/libdash/ -rf

    #libflv
    echo "copy libflv ${srcname}/libflv/${builddir}"
    mkdir -p $installdir/libflv/lib
    cp ${srcname}/libflv/${builddir}/lib* $installdir/libflv/lib/ -rf
    cp ${srcname}/libflv/include $installdir/libflv/ -rf

    #libhls
    echo "copy libhls ${srcname}/libhls/${builddir}"
    mkdir -p $installdir/libhls/lib
    cp ${srcname}/libhls/${builddir}/lib* $installdir/libhls/lib/ -rf
    cp ${srcname}/libhls/include $installdir/libhls/ -rf

    #libmkv
    echo "copy libmkv ${srcname}/libmkv/${builddir}"
    mkdir -p $installdir/libmkv/lib
    cp ${srcname}/libmkv/${builddir}/lib* $installdir/libmkv/lib/ -rf
    cp ${srcname}/libmkv/include $installdir/libmkv/ -rf

    #libmov
    echo "copy libmov ${srcname}/libmov/${builddir}"
    mkdir -p $installdir/libmov/lib
    cp ${srcname}/libmov/${builddir}/lib* $installdir/libmov/lib/ -rf
    cp ${srcname}/libmov/include $installdir/libmov/ -rf

    #libmpeg
    echo "copy libmpeg ${srcname}/libmpeg/${builddir}"
    mkdir -p $installdir/libmpeg/lib
    cp ${srcname}/libmpeg/${builddir}/lib* $installdir/libmpeg/lib/ -rf
    cp ${srcname}/libmpeg/include $installdir/libmpeg/ -rf

    #librtmp
    echo "copy librtmp ${srcname}/librtmp/${builddir}"
    mkdir -p $installdir/librtmp/lib
    cp ${srcname}/librtmp/${builddir}/lib* $installdir/librtmp/lib/ -rf
    cp ${srcname}/librtmp/include $installdir/librtmp/ -rf

    #librtp
    echo "copy librtp ${srcname}/librtp/${builddir}"
    mkdir -p $installdir/librtp/lib
    cp ${srcname}/librtp/${builddir}/lib* $installdir/librtp/lib/ -rf
    cp ${srcname}/librtp/include $installdir/librtp/ -rf

    #librtsp
    echo "copy librtsp ${srcname}/librtsp/${builddir}"
    mkdir -p $installdir/librtsp/lib
    cp ${srcname}/librtsp/${builddir}/lib* $installdir/librtsp/lib/ -rf
    cp ${srcname}/librtsp/include $installdir/librtsp/ -rf
    ##copy rtsp-server-internal.h
    cp ${srcname}/librtsp/source/server/rtsp-server-internal.h $installdir/librtsp/include -rf

    #libsip
    echo "copy libsip ${srcname}/libsip/${builddir}"
    mkdir -p $installdir/libsip/lib
    cp ${srcname}/libsip/${builddir}/lib* $installdir/libsip/lib/ -rf
    cp ${srcname}/libsip/include $installdir/libsip/ -rf
}


check_builddir
build_make
build_installdir

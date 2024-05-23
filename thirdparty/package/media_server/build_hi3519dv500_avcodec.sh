#! /bin/bash

basepath=$(cd `dirname $01`; pwd)

#sdk
srcpkg=avcodec.tar.gz
srcpkgbasename=avcodec
srcname=avcodec
srcdir=$basepath/${srcname}
installabsdir=$basepath/../../install/hi3519dv500/media_server/${srcname}
[ -d $installabsdir ] || mkdir -p $installabsdir
installdir=$(cd $installabsdir; pwd)

#debug/release
RELEASE=0
if [ x$RELEASE == x1 ]; then
	BUILD=release
else
	BUILD=debug
fi

platform=aarch64-v01c01-linux-gnu
#platform=
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
export CXXFLAGS='-mcpu=cortex-a55'

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
    make clean
    make ${buildargs} -j 16
    make install
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
    #avbsf
    echo "copy avbsf ${srcname}/avbsf/${builddir}"
    mkdir -p $installdir/avbsf/lib
    cp ${srcname}/avbsf/${builddir}/lib* $installdir/avbsf/lib/ -rf
    cp ${srcname}/avbsf/include $installdir/avbsf/ -rf

    #avcodec
    echo "copy avcodec ${srcname}/avcodec/${builddir}"
    mkdir -p $installdir/avcodec/lib
    cp ${srcname}/avcodec/${builddir}/lib* $installdir/avcodec/lib/ -rf
    cp ${srcname}/avcodec/include $installdir/avcodec/ -rf

    #avplayer
    # echo "copy avplayer ${srcname}/avplayer/${builddir}"
    # mkdir -p $installdir/avplayer/lib
    # cp ${srcname}/avplayer/${builddir}/lib* $installdir/avplayer/lib/ -rf
    # cp ${srcname}/avplayer/include $installdir/avplayer/ -rf

    #ffutils
    # echo "copy ffutils ${srcname}/ffutils/${builddir}"
    # mkdir -p $installdir/ffutils/lib
    # cp ${srcname}/ffutils/${builddir}/lib* $installdir/ffutils/lib/ -rf
    # cp ${srcname}/ffutils/include $installdir/ffutils/ -rf

    #h264
    echo "copy h264 ${srcname}/h264/${builddir}"
    mkdir -p $installdir/h264/lib
    cp ${srcname}/h264/${builddir}/lib* $installdir/h264/lib/ -rf
    cp ${srcname}/h264/include $installdir/h264/ -rf

    #h265
    echo "copy h265 ${srcname}/h265/${builddir}"
    mkdir -p $installdir/h265/lib
    cp ${srcname}/h265/${builddir}/lib* $installdir/h265/lib/ -rf
    cp ${srcname}/h265/include $installdir/h265/ -rf

    #libavo
    # echo "copy libavo ${srcname}/libavo/${builddir}"
    # mkdir -p $installdir/libavo/lib
    # cp ${srcname}/libavo/${builddir}/lib* $installdir/libavo/lib/ -rf
    # cp ${srcname}/libavo/include $installdir/libavo/ -rf
}


check_builddir
build_make
build_installdir

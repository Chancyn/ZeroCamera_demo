#! /bin/bash
basepath=$(cd `dirname $0`; pwd)
cd $basepath
#sdk
srcpkg=sdk.tar.gz
srcpkgbasename=sdk
srcname=sdk
srcdir=$basepath/${srcname}

function check_builddir_sdk()
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

avcodecsrcpkg=avcodec.tar.gz
avcodecsrcpkgbasename=avcodec
avcodecsrcname=avcodec
avcodecsrcdir=$basepath/${avcodecsrcname}
function check_builddir_avcodec()
{
    if [ ! -d $avcodecsrcdir ]; then
        #echo "error ${avcodecsrcdir} not exist"
        if [ -e $avcodecsrcpkg ]; then
        echo "error ${avcodecsrcdir} not exist,unpack package ${avcodecsrcpkg}"
        tar -zxvf ${avcodecsrcpkg}
        mv ${avcodecsrcpkgbasename} ${avcodecsrcdir}
        else
        echo "error ${avcodecsrcpkg} not exist"
        exit -1;
        fi
    fi
}

#media-server
mssrcpkg=media-server.tar.gz
mssrcpkgbasename=media-server
mssrcname=media-server
mssrcdir=$basepath/${mssrcname}

function check_builddir_mediaserver()
{
    if [ ! -d $mssrcdir ]; then
        #echo "error ${mssrcdir} not exist"
        if [ -e $mssrcpkg ]; then
        echo "error ${srcdir} not exist,unpack package ${mssrcpkg}"
        tar -zxvf ${mssrcpkg}
        mv ${mssrcpkgbasename} ${mssrcdir}
        else
        echo "error ${mssrcpkg} not exist"
        exit -1;
        fi
    fi
}

check_builddir_sdk
check_builddir_avcodec
check_builddir_mediaserver

echo "----------------------------------"
source ./build_x86_64_sdk.sh
echo "sdk end----------------------------------"
source ./build_x86_64_avcodec.sh
echo "avcode end----------------------------------"
source ./build_x86_64_media_server.sh
echo "end----------------------------------"
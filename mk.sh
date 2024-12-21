#! /bin/bash
source base.sh
basepath=$(cd `dirname $0`; pwd)
cd $basepath

#soc="x86_64"

#CMake编译工作目录生成文件的地方
builddir=$basepath/build
outputdir=$basepath/output/${soc}
rundir=/nfsroot/run/${soc}
thirdinstalldir=$basepath/thirdparty/install/${soc}
thirdpackagedir=$basepath/thirdparty/package
echo_debug "builddir=${builddir}"
echo_debug "outputdir=${outputdir}"

toolchainfile_prefix=toolchain/linux_toolchain
toolchainfile_prefix=toolchain/linux_toolchain
toolchainfile=linux_toolchain_x84_64.camke

function build_cmake(){
    # Initial directory
    rm -rf $builddir  #每次都重新编译，删除build文件
    mkdir -p $builddir #创建build文件

    pushd $builddir
    # Run cmake
if [ "${cross}" = "1" ]; then
    toolchainfile=${toolchainfile_prefix}_cross_${soc}.cmake
else
    toolchainfile=${toolchainfile_prefix}_${soc}.cmake
fi
    echo ${toolchainfile}
    cmake -DCMAKE_TOOLCHAIN_FILE=${toolchainfile} $basepath  #编译cmake
    popd
}

function build_make(){
    pushd $builddir
    # Run make
    #make
    #bear --output /home/zhoucc/work/study/cmake/test/compile_commands.json -- make

    bear --output ${basepath}/compile_commands.json -- make VERBOSE=on -j 16
    #make VERBOSE=on -j 16
    make install
    popd
}

function build_make_debug(){
    pushd $builddir
    # Run make
    #make
    bear --output ${basepath}/compile_commands.json -- make VERBOSE=on -j 16
    #make VERBOSE=on -j 16
    make install
    popd
}

function build_make_append(){
    pushd $builddir
    # Run make
    #make
    bear --output ${basepath}/compile_commands.json -- make VERBOSE=on
    #make install
    popd
}

function submodule_media_server(){
    if [ ! -d ${thirdpackagedir}/media_server/media-server ]; then
    echo_warn "${thirdinstalldir}/media_server/media-server submodule not exist"
    git submodule init thirdparty/package/media_server/media-server
    git submodule update
    else
    echo_info "${thirdinstalldir}media_server/media-server exist"
    fi

}

function build_check_thirdparty(){
    if [ ! -d ${thirdinstalldir} ]; then
    echo_warn "${thirdinstalldir} not exist mkdir"
    mkdir ${thirdinstalldir}
    fi

    # media_server
    if [ ! -d ${thirdinstalldir}/media_server ]; then
    echo_warn "media_server not exist build"
    pushd ${thirdpackagedir}/media_server
    source ./build_${soc}.sh
    popd 
    fi

    # nng
    if [ ! -d ${thirdinstalldir}/nng ]; then
    echo_warn "nng not exist build"
    pushd ${thirdpackagedir}/nng
    source ./build_${soc}.sh
    popd 
    fi

    #openssl
    if [ ! -d ${thirdinstalldir}/openssl ]; then
    echo_warn "openssl not exist build"
    pushd ${thirdpackagedir}/openssl
    source ./build_${soc}.sh
    popd 
    fi

    #spdlog
    if [ ! -d ${thirdinstalldir}/spdlog ]; then
    echo_warn "spdlog not exist build"
    pushd ${thirdpackagedir}/spdlog
    source ./build_${soc}.sh
    popd 
    fi

    #srt
    if [ ! -d ${thirdinstalldir}/srt ]; then
    pushd ${thirdpackagedir}/srt
    source ./build_${soc}.sh
    popd 
    fi

    #v4l2cpp
    if [ ! -d ${thirdinstalldir}/v4l2cpp ]; then
    pushd ${thirdpackagedir}/v4l2cpp
    source ./build_${soc}.sh
    popd 
    fi
}

function build_copy_thirdparty(){
    # copy nng
    cp ${thirdinstalldir}/nng/lib/lib*.so* ${outputdir}/lib
    cp ${thirdinstalldir}/media_server/sdk/libaio/lib/lib*.so* ${outputdir}/lib
    cp ${thirdinstalldir}/srt/lib/lib*.so* ${outputdir}/lib
    #cp ${thirdinstalldir}/openssl/lib/lib*.so* ${outputdir}/lib
}

function build_copy2rundir(){
    # copy nng
    cp ${outputdir}/bin/* ${rundir}/bin
    cp ${outputdir}/lib/lib*.so* ${rundir}/lib
}

echo "----------------------------------"
#enable_debug
export WITH_DEBUG=y

# export WITH_ZC_APP=1
# export WITH_ZC_TEST=0

build_check_thirdparty
#build_cmake
#build_make
#build_make_debug
#build_make_append
#build_copy_thirdparty
#build_copy2rundir
echo_info "mk soc=${soc} cross=${cross} end"
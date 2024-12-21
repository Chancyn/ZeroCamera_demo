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
    echo_debug "---------------cmake into-------------------"
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
    echo_debug "---------------cmake end-------------------"
}

function build_make(){
    echo_debug "---------------make into-------------------"
    pushd $builddir
    # Run make
    #make
    #bear --output /home/zhoucc/work/study/cmake/test/compile_commands.json -- make

    bear --output ${basepath}/compile_commands.json -- make VERBOSE=on -j 16
    #make VERBOSE=on -j 16
    make install
    popd
    echo_debug "---------------make end-------------------"
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
    echo_debug "---------------submodule into-------------------"
    echo_warn "${thirdinstalldir}/media_server/media-server submodule not exist"
    git submodule init thirdparty/package/media_server/media-server
    git submodule update
    pushd thirdparty/package/media_server/media-server/
    git checkout master
    popd
    echo_debug "---------------submodule end-------------------"
    else
    echo_info "${thirdinstalldir}media_server/media-server exist"
    fi

}

function build_check_thirdparty(){
    echo_debug "---------------check thirdparty into-------------------"
    if [ ! -d ${thirdinstalldir} ]; then
    echo_warn "${thirdinstalldir} not exist mkdir"
    mkdir ${thirdinstalldir}
    fi

    # media_server
    if [ ! -d ${thirdinstalldir}/media_server ]; then
    echo_debug "---------------check media_server into-------------------"
    echo_warn "media_server not exist build"
    submodule_media_server
    pushd ${thirdpackagedir}/media_server
    ./build_${soc}.sh
    if [ $? -ne 0 ]; then
        rm ${thirdinstalldir}/media_server -rf
        echo_err "error: build media_server with exit status $?"
        exit 1
    fi
    popd 
    echo_debug "---------------check media_server end-------------------"
    fi

    # nng
    if [ ! -d ${thirdinstalldir}/nng ]; then
    echo_debug "---------------check nng into-------------------"
    echo_warn "nng not exist build"
    pushd ${thirdpackagedir}/nng
    ./build_${soc}.sh
    if [ $? -ne 0 ]; then
        rm ${thirdinstalldir}/nng -rf
        echo_err "error: build nng with exit status $?"
        exit 1
    fi
    popd
    echo_debug "---------------check nng end-------------------"
    fi

    #openssl
    if [ ! -d ${thirdinstalldir}/openssl ]; then
    echo_debug "---------------check openssl into-------------------"
    echo_warn "openssl not exist build"
    pushd ${thirdpackagedir}/openssl
    ./build_${soc}.sh
    if [ $? -ne 0 ]; then
        rm ${thirdinstalldir}/openssl -rf
        echo_err "error: build openssl with exit status $?"
        exit 1
    fi
    popd 
    echo_debug "---------------check openssl end-------------------"
    fi

    #spdlog
    if [ ! -d ${thirdinstalldir}/spdlog ]; then
    echo_debug "---------------check spdlog into-------------------"
    echo_warn "spdlog not exist build"
    pushd ${thirdpackagedir}/spdlog
    ./build_${soc}.sh
    if [ $? -ne 0 ]; then
        rm ${thirdinstalldir}/spdlog -rf
        echo_err "error: build spdlog with exit status $?"
        exit 1
    fi
    popd
    echo_debug "---------------check spdlog end-------------------"
    fi

    #srt
    if [ ! -d ${thirdinstalldir}/srt ]; then
    pushd ${thirdpackagedir}/srt
    ./build_${soc}.sh
    if [ $? -ne 0 ]; then
        rm ${thirdinstalldir}/srt -rf
        echo_err "error: build srt with exit status $?"
        exit 1
    fi
    popd 
    fi

    #v4l2cpp
    if [ ! -d ${thirdinstalldir}/v4l2cpp ]; then
    echo_debug "----------------------------------"
    pushd ${thirdpackagedir}/v4l2cpp
    source ./build_${soc}.sh
    if [ $? -ne 0 ]; then
        rm ${thirdinstalldir}/v4l2cpp -rf
        echo_err "error: build v4l2cpp with exit status $?"
        exit 1
    fi
    echo_debug "----------------------------------"
    popd 
    fi

    echo_debug "---------------check thirdparty end-------------------"
}

function build_copy_thirdparty(){
    echo_debug "---------------copy thirdparty into-------------------"
    # copy nng
    cp ${thirdinstalldir}/nng/lib/lib*.so* ${outputdir}/lib
    cp ${thirdinstalldir}/media_server/sdk/libaio/lib/lib*.so* ${outputdir}/lib
    cp ${thirdinstalldir}/srt/lib/lib*.so* ${outputdir}/lib
    #cp ${thirdinstalldir}/openssl/lib/lib*.so* ${outputdir}/lib
    echo_debug "---------------copy thirdparty end-------------------"
}

function build_copy2rundir(){
    if [ ! -d ${outputdir} ]; then
    echo_debug "---${outputdir} not exist mkdir---------------------"
    mkdir -p ${outputdir}
    fi
    # copy nng
    echo_debug "---------------pls ----------------------------------"
    echo_warn "------------------------------------------------------"
    echo_warn "export LD_LIBRARY_PATH=${rundir}/lib:$LD_LIBRARY_PATH"
    echo_warn "------------------------------------------------------"
    cp ${outputdir}/bin/* ${rundir}/bin
    cp ${outputdir}/lib/lib*.so* ${rundir}/lib
    echo_debug "------------------------------------------"
}

echo_debug "----------------------------------"
#enable_debug
export WITH_DEBUG=y

# export WITH_ZC_APP=1
# export WITH_ZC_TEST=0

build_check_thirdparty
echo_info "check_thirdparty ok"
build_cmake
build_make
#build_make_debug
#build_make_append
build_copy_thirdparty
build_copy2rundir
echo_info "mk soc=${soc} cross=${cross} end"
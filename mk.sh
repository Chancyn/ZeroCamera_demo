#! /bin/bash
source base.sh
basepath=$(cd `dirname $0`; pwd)
cd $basepath

#soc="x86_64"
#buildcmd=make
#是否交叉编译
cross="0"
# 编译参数
build_VERBOSE="0"

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

function build_cmake_clean(){
    echo_debug "---------------cmake clean into-------------------"
    # Initial directory
    rm -rf $builddir  #每次都重新编译，删除build文件
    echo_debug "---------------cmake clean end-------------------"
}

function build_cmake(){
    echo_debug "---------------cmake into-------------------"
if [ ! -d ${builddir}/Makefile ]; then
    if [ ! -d ${builddir} ]; then
        echo_debug "-------$builddir not exist mkdir--------------"
        mkdir -p $builddir #创建build文件
    fi

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
    echo_debug "---------------make end-------------------------"
fi
}

function build_make(){
    echo_debug "---------------make into-----------------------------"
    pushd $builddir
    # Run make
if [ "${build_VERBOSE}" = "1" ]; then
    bear --output ${basepath}/compile_commands.json -- make VERBOSE=on -j 16
else
    bear --output ${basepath}/compile_commands.json -- make -j 16
fi
    make install
    popd
    echo_debug "---------------make end-------------------------"
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
    echo_debug "------------------------------------------------------"
    pushd ${thirdpackagedir}/v4l2cpp
    source ./build_${soc}.sh
    if [ $? -ne 0 ]; then
        rm ${thirdinstalldir}/v4l2cpp -rf
        echo_err "error: build v4l2cpp with exit status $?"
        exit 1
    fi
    echo_debug "------------------------------------------------------"
    popd 
    fi

    echo_debug "---------------check thirdparty end-------------------"
}

function build_copy_thirdparty(){
    echo_debug "---------------copy thirdparty into------------------"
    # copy nng
    cp ${thirdinstalldir}/nng/lib/lib*.so* ${outputdir}/lib
    cp ${thirdinstalldir}/media_server/sdk/libaio/lib/lib*.so* ${outputdir}/lib
    cp ${thirdinstalldir}/srt/lib/lib*.so* ${outputdir}/lib
    #cp ${thirdinstalldir}/openssl/lib/lib*.so* ${outputdir}/lib
    echo_debug "---------------copy thirdparty end-------------------"
}

function build_copy2rundir(){
    if [ ! -d ${rundir} ]; then
    echo_debug "---${rundir} not exist pls run"
    echo_debug "sudo mkdir -p ${rundir}/bin"
    echo_debug "sudo mkdir -p ${rundir}/lib"
    mkdir -p ${rundir}/bin
    mkdir -p ${rundir}/lib
    fi
    # copy nng
    echo_warn "------------------------------------------------------"
    echo_debug "---------------pls run-------------------------------"
    echo_warn "export LD_LIBRARY_PATH=${rundir}/lib:\$LD_LIBRARY_PATH"
    echo_warn "------------------------------------------------------"
    cp ${outputdir}/bin/* ${rundir}/bin
    cp ${outputdir}/lib/lib*.so* ${rundir}/lib
    echo_debug "------------------------------------------------------"
}

function build_thirdparty(){
    echo_debug "---------------build thirdparty into-------------------"

    echo_debug "---------------build thirdparty end-------------------"
}

function buildcmd_process(){
echo_debug "------------------------------------------------------"
if [ "${buildcmd}" = "make" ]; then
build_check_thirdparty
echo_info "check_thirdparty ok"
build_cmake
build_make
build_copy_thirdparty
build_copy2rundir
elif [ "${buildcmd}" = "clean" ]; then
build_cmake_clean
elif [ "${buildcmd}" = "pack" ]; then
build_copy2rundir
elif [ "${buildcmd}" = "third" ]; then
build_thirdparty
fi

echo_info "mk soc=${soc} cross=${cross} end"
echo_debug "------------------------------------------------------"
}

buildcmd_process
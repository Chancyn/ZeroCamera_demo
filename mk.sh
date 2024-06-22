#! /bin/bash
basepath=$(cd `dirname $0`; pwd)
cd $basepath

#soc="x64"

#CMake编译工作目录生成文件的地方
builddir=$basepath/build/${soc}
outputdir=$basepath/output/${soc}
thirdinstalldir=$basepath/thirdparty/install/${soc}
echo "builddir=${builddir}"
echo "outputdir=${outputdir}"

function build_cmake(){
    # Initial directory
    rm -rf $builddir  #每次都重新编译，删除build文件
    mkdir -p $builddir #创建build文件

    pushd $builddir
    # Run cmake
    cmake $basepath  #编译cmake
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

function build_copy_thirdparty(){
    # copy nng
    cp ${thirdinstalldir}/nng/lib/lib*.so* ${outputdir}/lib
}

echo "----------------------------------"
#enable_debug
export WITH_DEBUG=y

# export WITH_ZC_APP=1
# export WITH_ZC_TEST=0

build_cmake
build_make
#build_make_debug
#build_make_append
build_copy_thirdparty
echo "mk soc=${soc} end"
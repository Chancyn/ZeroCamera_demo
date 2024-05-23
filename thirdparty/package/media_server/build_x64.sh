#! /bin/bash
basepath=$(cd `dirname $0`; pwd)
cd $basepath

echo "----------------------------------"
source ./build_x64_sdk.sh
echo "sdk end----------------------------------"
source ./build_x64_avcodec.sh
echo "avcode end----------------------------------"
source ./build_x64_media_server.sh
echo "end----------------------------------"
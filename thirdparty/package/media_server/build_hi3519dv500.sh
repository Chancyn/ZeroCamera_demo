#! /bin/bash
basepath=$(cd `dirname $0`; pwd)
cd $basepath

echo "----------------------------------"
source ./build_hi3519dv500_sdk.sh
echo "sdk end----------------------------------"
source ./build_hi3519dv500_avcodec.sh
echo "avcode end----------------------------------"
source ./build_hi3519dv500_media_server.sh
echo "end----------------------------------"
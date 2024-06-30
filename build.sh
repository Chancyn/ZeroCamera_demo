#! /bin/bash
basepath=$(cd `dirname $0`; pwd)
cd $basepath

soc="x64"
export soc=${soc}

function parse_soc(){
if [ `expr match "$1" "soc="` -eq 4 ];then
soc=${1#soc=}
fi
case ${soc} in
hi3519dv500 )
echo "soc type is ${soc}"
export soc=${soc}
;;
ss928v100 )
echo "soc type is ${soc}"
export soc=${soc}
;;
rk3588 )
echo "soc type is ${soc}"
export soc=${soc}
;;
x64 )
echo "soc type is ${soc}"
export soc=${soc}
;;
*)
echo "soc type is ${soc}"
echo -e "badddd, exit!!!\nusage:\n ./build.sh  soc=hi3519dv500"
exit 0;
esac
}

if [ $# -eq 1 ];then
  parse_soc $1
fi
echo "----------------------------------"
echo "build soc=${soc}"
source ./mk.sh


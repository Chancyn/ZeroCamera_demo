#! /bin/bash
source base.sh
basepath=$(cd `dirname $0`; pwd)
cd $basepath

#soc="x86_64"
soc=$(uname -m)
cross="0"
export soc=${soc}
export cross=${cross}

function parse_soc(){
if [ `expr match "$1" "soc="` -eq 4 ];then
soc=${1#soc=}
fi
case ${soc} in
hi3519dv500 )
echo_info "soc type is ${soc}"
export soc=${soc}
export cross=1
;;
ss928v100 )
echo_info "soc type is ${soc}"
export soc=${soc}
export cross=1
;;
rk3588 )
echo_info "soc type is ${soc}"
export soc=${soc}
;;
x86_64 )
echo_info "soc type is ${soc}"
export soc=${soc}
;;
*)
echo_debug "soc type is ${soc}"
echo_er -e "badddd, exit!!!\nusage:\n ./build.sh  soc=hi3519dv500 [cross=0/1]"
exit 0;
esac
}

function parse_cross(){
if [ `expr match "$1" "cross="` -eq 6 ];then
cross=${1#cross=}
fi
case ${cross} in
0 )
echo_debug "clear cross flag = ${cross}"
export cross=${cross}
;;
1 )
echo_debug "set cross flag = ${cross}"
export cross=${cross}
;;
*)
echo_debug "cross type is ${cross}"
echo_err -e "badddd, exit!!!\nusage:\n ./build.sh  soc=hi3519dv500 [cross=0/1]"
exit 0;
esac
}

if [ $# -gt 0 ];then
  parse_soc $1
fi

if [ $# -gt 1 ];then
  parse_cross $2
fi

echo_debug "----------------------------------"
echo_info "build soc=${soc} cross=${cross}"
time source ./mk.sh
echo_debug "----------------------------------"

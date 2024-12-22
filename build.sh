#! /bin/bash
source base.sh
basepath=$(cd `dirname $0`; pwd)
cd $basepath

#input_args="soc=x86_64 cmd=make"
#soc="x86_64"
soc=$(uname -m)
export soc=${soc}

buildcmd="make"
export buildcmd=${buildcmd}

# 是否交叉编译
cross="0"

# shell脚本调试信息
export shell_debug=1

function parse_soc(){
soc=$1
echo_info "${1} ${soc}"
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
export cross=1
;;
x86_64 )
echo_info "soc type is ${soc}"
export soc=${soc}
;;
*)
echo_debug "soc type is ${soc}"
echo_err -e "badddd, exit!!!\nusage:\n ./build.sh  soc=hi3519dv500 [-[c|m|p]]"
exit 1;
esac
}

function parse_buildcmd(){
buildcmd=${1}
case ${buildcmd} in
clean|c )
echo_debug "buildcmd clean = ${buildcmd}"
export buildcmd=clean
;;
make|m )
echo_debug "buildcmd  make = ${buildcmd}"
export buildcmd=${buildcmd}
export buildcmd=make
;;
pack|p )
echo_debug "buildcmd package = ${buildcmd}"
export buildcmd=pack
;;
third|t )
echo_debug "buildcmd third = ${buildcmd}"
export buildcmd=third
;;
*)
echo_debug "buildcmd make ${buildcmd}"
echo_err -e "badddd, exit!!!\nusage:\n ./build.sh  soc=hi3519dv500 [-[c|m|p|t]]"
echo_warn "-c:clean; -m:make; -p:package; -t:third"
exit 1;
esac
}

 function parse_args(){
# 声明一个关联数组来存储解析后的键值对
declare -A params
 
  # 使用for循环遍历所有输入参数
  for arg in "$@"; do
    # 使用IFS='='来分割每个键值对
    IFS='=' read -r key value <<< "$arg"
    # 去除值两边的空格（如果有的话），这不是严格必要的，除非你知道值可能包含前导或尾随空格
    # value="${value##*( )}"
    # value="${value%%*( )}"
    # 但是，由于我们是在 <<< 字符串重定向中使用的 read，通常不会有前导或尾随空格问题
    # 将键值对存储到关联数组中
    params["$key"]="$value"
  done
  
  # 现在，我们可以从关联数组中提取并使用键值对了
  soc_value=${params[soc]}
  echo_debug "soc_val: ${soc_value}"

  cmd_value=${params[cmd]}
  echo_debug "cmd_val: ${cmd_value}"

  if [ ! -z "$soc_value" ]; then
    parse_soc ${soc_value}
  fi

  if [ ! -z "$cmd_value" ]; then
    parse_buildcmd ${cmd_value}
  fi
}

echo_debug "------------------------------------------------------"
parse_args "$@"
echo_info "build soc=${soc} buildcmd=${buildcmd} cross=${cross} "
time ./mk.sh
echo_info "build soc=${soc} buildcmd=${buildcmd} cross=${cross} "
echo_debug "------------------------------------------------------"
